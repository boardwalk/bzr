/*
 * Bael'Zharon's Respite
 * Copyright (C) 2014 Daniel Skorupski
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */
#include "net/Session.h"
#include "net/SessionManager.h"
#include "net/Socket.h"
#include "BinReader.h"
#include "BinWriter.h"
#include "Core.h"
#include "Log.h"

enum PacketFlags
{
    kRetransmission    = 0x00000001,
    kEncryptedChecksum = 0x00000002,
    kBlobFragments     = 0x00000004,
    kServerSwitch      = 0x00000100, // CServerSwitchStruct (?)
    kUnknown1          = 0x00000200, // sockaddr_in (7, kDisposable|kExclusive|kNotConn)
    kUnknown2          = 0x00000400, // EmptyHeader (7, kDisposable|kExclusive|kNotConn)
    kReferral          = 0x00000800, // CReferralStruct (?)
    kRequestRetransmit = 0x00001000, // SeqIDList (33, kDisposable|kExclusive|kShouldPiggyBack|kHighPriority)
    kRejectRetransmit  = 0x00002000, // SeqIDList (33, kDisposable|kExclusive|kShouldPiggyBack|kHighPriority)
    kAckSequence       = 0x00004000, // unsigned long (1, kDisposable)
    kDisconnect        = 0x00008000, // EmptyHeader (3, kDisposable|kExclusive)
    kLogon             = 0x00010000,
    kReferred          = 0x00020000, // uint64_t (7, kDisposable|kExclusive|kNotConn)
    kConnectRequest    = 0x00040000, // CConnectHeader (?)
    kConnectResponse   = 0x00080000, // uint64_t (20000007, kDisposable|kExclusive|kNotConn|kEncrypted)
    kNetError1         = 0x00100000, // PackObjHeader<NetError> (7, kDisposable|kExclusive|kNotConn)
    kNetError2         = 0x00200000, // PackObjHeader<NetError> (2, kExclusive)
    kCICMDCommand      = 0x00400000, // CICMDCommandStruct (7, kDisposable|kExclusive|kNotConn)
    kTimeSync          = 0x01000000, // CTimeSyncHeader (?)
    kEchoRequest       = 0x02000000, // CEchoRequestHeader (?)
    kEchoResponse      = 0x04000000, // CEchoResponseHeader (?)
    kFlow              = 0x08000000  // CFloatStruct (?)
};

PACK(struct PacketHeader {
    uint32_t sequence;
    uint32_t flags;
    uint32_t checksum;
    uint16_t netId;
    uint16_t time;
    uint16_t size;
    uint16_t iteration;
});

PACK(struct BlobHeader {
    uint64_t id;
    uint16_t count;
    uint16_t size;
    uint16_t index;
    uint16_t queueId;
});

static const chrono::milliseconds kLogonPacketDelay(300);
static const chrono::milliseconds kReferredPacketDelay(300);
static const chrono::milliseconds kConnectResponsePacketDelay(300);
static const chrono::milliseconds kPingPacketDelay(2000);
static const size_t kMaxPeriodicSent = 10;

static uint32_t checksum(const void* data, size_t size)
{
    uint32_t result = static_cast<uint32_t>(size) << 16;

    for(size_t i = 0; i < size / 4; i++)
    {
        result += static_cast<const uint32_t*>(data)[i];
    }

    int shift = 24;

    for(size_t i = (size / 4) * 4; i < size; i++)
    {
        result += static_cast<const uint8_t*>(data)[i] << shift;
        shift -= 8;
    }

    return result;
}

static uint32_t checksumHeader(const PacketHeader* header)
{
    PacketHeader* header_nc = const_cast<PacketHeader*>(header);

    uint32_t origChecksum = header->checksum;
    header_nc->checksum = 0xBADD70DD;

    uint32_t result = checksum(header, sizeof(PacketHeader));

    header_nc->checksum = origChecksum;

    return result;
}

static uint32_t checksumContent(const PacketHeader* header, const void* data)
{
    if(header->flags & kEncryptedChecksum)
    {
        BinReader reader(data, header->size);

        if(header->flags & kServerSwitch)
        {
            reader.readRaw(8);
        }

        if(header->flags & kRequestRetransmit)
        {
            uint32_t nseq = reader.readInt();
            reader.readRaw(nseq * sizeof(uint32_t));
        }

        if(header->flags & kRejectRetransmit)
        {
            uint32_t nseq = reader.readInt();
            reader.readRaw(nseq * sizeof(uint32_t));
        }

        if(header->flags & kCICMDCommand)
        {
            reader.readRaw(8);
        }

        if(header->flags & kTimeSync)
        {
            reader.readRaw(8);
        }

        if(header->flags & kEchoRequest)
        {
            reader.readRaw(4);
        }

        if(header->flags & kEchoResponse)
        {
            reader.readRaw(8);
        }

        if(header->flags & kFlow)
        {
            reader.readRaw(6);
        }

        uint32_t result = checksum(data, reader.position());

        if(header->flags & kBlobFragments)
        {
            while(reader.remaining() != 0)
            {
                const BlobHeader* blobHeader = reader.readPointer<BlobHeader>();

                reader.readRaw(blobHeader->size - sizeof(BlobHeader));

                result += checksum(blobHeader, blobHeader->size);
            }
        }

        assert(reader.remaining() == 0);

        return result;
    }

    return checksum(data, header->size);
}

static uint32_t checksumPacket(const Packet& packet, ChecksumXorGenerator& xorGen)
{
    const PacketHeader* header = reinterpret_cast<const PacketHeader*>(packet.data.data());
    const void* data = reinterpret_cast<const uint8_t*>(packet.data.data()) + sizeof(PacketHeader);

    uint32_t xorVal = (header->flags & kEncryptedChecksum) ? xorGen.get(header->sequence) : 0;
    return checksumHeader(header) + checksumContent(header, data) ^ xorVal;
}

Session::Session(SessionManager& manager,
    Address address,
    string accountName,
    string accountKey) :
    manager_(manager),
    address_(address),
    state_(State::kLogon),
    nextPeriodic_(net_clock::now() + kLogonPacketDelay),
    numPeriodicSent_(1),
    accountName_(move(accountName)),
    accountKey_(move(accountKey))
{
    sendLogon();
}

Session::Session(SessionManager& manager,
    Address address,
    uint64_t cookie) :
    manager_(manager),
    address_(address),
    state_(State::kReferred),
    nextPeriodic_(net_clock::now() + kConnectResponsePacketDelay),
    numPeriodicSent_(1),
    cookie_(cookie)
{
    sendReferred();
}

void Session::handle(const Packet& packet)
{
    BinReader reader(packet.data.data(), packet.size);

    const PacketHeader* header = reader.readPointer<PacketHeader>();

    if(header->size != reader.remaining())
    {
        LOG(Net, Warn) << address_ << " dropping packet, packet size is " << reader.remaining() << ", packet has " << header->size << "\n";
        return;
    }

    const uint32_t calcChecksum = checksumPacket(packet, serverXorGen_);

    if(header->checksum != calcChecksum)
    {
        LOG(Net, Warn) << address_ << " dropping packet, calc checksum is " << hexn(calcChecksum) << ", packet has " << hexn(header->checksum) << "\n";
        return;
    }

    if(state_ == State::kLogon || state_ == State::kReferred)
    {
        if(header->flags != kConnectRequest)
        {
            LOG(Net, Warn) << address_ << "dropping packet, flags should be " << hexn(static_cast<uint32_t>(kConnectRequest)) << ", packet has " << hexn(header->flags) << "\n";
            return;
        }

        handleConnect(header, reader);

        state_ = State::kConnectResponse;
        numPeriodicSent_ = 0;
        sendConnectResponse();
    }
    else if(state_ == State::kConnectResponse)
    {
        LOG(Net, Info) << address_ << " transitioning to connected\n";

        state_ = State::kConnected;
        numPeriodicSent_ = 0;
        return handle(packet);
    }
    else if(state_ == State::kConnected)
    {
        if(header->sequence <= serverSequence_)
        {
            LOG(Net, Warn) << address_ << " dropping packet, server sequence is " << serverSequence_ << ", packet has " << header->sequence << "\n";
            return;
        }

        if(header->netId != serverNetId_)
        {
            LOG(Net, Warn) << address_ << " dropping packet, server net id is " << serverNetId_ << ", packet has " << header->netId << "\n";
            return;
        }

        if(header->iteration != iteration_)
        {
            LOG(Net, Warn) << address_ << " dropping packet, iteration is " << iteration_ << ", packet has " << header->iteration << "\n";
            return;
        }

        // record that we've received the packet
        serverPackets_.insert(header->sequence);
        advanceServerSequence();

        uint32_t flags = header->flags & ~(kRetransmission | kEncryptedChecksum);

        if(flags == kServerSwitch)
        {
            handleServerSwitch(reader);
        }
        else if(flags == kReferral)
        {
            handleReferral(reader);
        }
        else
        {
            if(flags & kRequestRetransmit)
            {
                handleRequestRetransmit(reader);
                flags &= ~kRequestRetransmit;
            }

            if(flags & kRejectRetransmit)
            {
                handleRejectRetransmit(reader);
                flags &= ~kRejectRetransmit;
            }

            if(flags & kAckSequence)
            {
                handleAckSequence(reader);
                flags &= ~kAckSequence;
            }

            if(flags & kTimeSync)
            {
                handleTimeSync(reader);
                flags &= ~kTimeSync;
            }

            if(flags & kEchoResponse)
            {
                handleEchoResponse(reader);
                flags &= ~kEchoResponse;
            }

            if(flags & kFlow)
            {
                handleFlow(reader);
                flags &= ~kFlow;
            }

            if(flags & kBlobFragments)
            {
                handleBlobFragments(reader);
                flags &= ~kBlobFragments;
            }

            if(flags != 0)
            {
                LOG(Net, Error) << address_ << " unhandled flags " << hexn(flags) << " in packet\n";
                throw runtime_error("unhandled flags");
            }
        }
    }

    if(reader.remaining() != 0)
    {
        LOG(Net, Error) << address_ << " unconsumed data of " << reader.remaining() << " bytes in packet\n";
        throw runtime_error("unconsumed data");
    }
}

void Session::tick(net_time_point now)
{
    if(nextPeriodic_ > now)
    {
        return;
    }

    if(state_ == State::kLogon)
    {
        sendLogon();
    }
    else if(state_ == State::kReferred)
    {
        sendReferred();
    }
    else if(state_ == State::kConnectResponse)
    {
        sendConnectResponse();
    }
    else
    {
        // TODO send ping, flow, time sync, ack
    }
}

Address Session::address() const
{
    if(state_ == State::kConnectResponse || state_ == State::kConnected)
    {
        return Address(address_.ip(), address_.port() + 1);
    }

    return address_;
}

bool Session::dead() const
{
    return numPeriodicSent_ > kMaxPeriodicSent;
}

net_time_point Session::nextTick() const
{
    return nextPeriodic_;
}

void Session::sendLogon()
{
    Packet packet;
    BinWriter writer(packet.data.data(), packet.data.size());

    // construct packet
    writer.skip(sizeof(PacketHeader));
    writer.writeString("1802"); // NetVersion
    size_t authDataLenOff = writer.position();
    writer.skip(sizeof(uint32_t));
    writer.writeInt(0x40000002); // GLSUserNameTicket_NetAuthType
    writer.writeInt(0);
    writer.writeInt(static_cast<uint32_t>(time(NULL)));
    writer.writeString(accountName_);
    writer.writeInt(0);
    writer.writeInt(static_cast<uint32_t>(accountKey_.size()));
    writer.writeRaw(accountKey_.data(), accountKey_.size());

    // fill in auth data len
    writer.seek(authDataLenOff);
    writer.writeInt(static_cast<uint32_t>(packet.size - writer.position()));

    // fill in packet header
    PacketHeader header;
    header.sequence = 0;
    header.flags = PacketFlags::kLogon;
    header.checksum = 0;
    header.netId = 0;
    header.time = 0;
    header.size = static_cast<uint16_t>(packet.size - sizeof(PacketHeader));
    header.iteration = 0;

    writer.seek(0);
    writer.writeRaw(&header, sizeof(header));

    // calculate checksum and rewrite header
    header.checksum = checksumPacket(packet, clientXorGen_);

    writer.seek(0);
    writer.writeRaw(&header, sizeof(header));

    // send packet
    manager_.send(packet);

    LOG(Net, Info) << address_ << " sent logon\n";
}

void Session::sendReferred()
{
    Packet packet;
    BinWriter writer(packet.data.data(), packet.data.size());

    // construct packet
    PacketHeader header;
    header.sequence = 0;
    header.flags = kReferred;
    header.checksum = 0;
    header.netId = 0;
    header.time = 0;
    header.size = sizeof(uint64_t);
    header.iteration = 0;

    writer.writeRaw(&header, sizeof(header));
    writer.writeLong(cookie_);

    // calc checksum and rewrite header
    header.checksum = checksumPacket(packet, clientXorGen_);

    writer.seek(0);
    writer.writeRaw(&header, sizeof(header));

    // send packet
    manager_.send(packet);
    nextPeriodic_ = net_clock::now() + kReferredPacketDelay;
    numPeriodicSent_++;

    LOG(Net, Info) << address_ << " sent referred\n";
}

void Session::sendConnectResponse()
{
    assert(state_ == State::kConnectResponse);

    Packet packet;
    BinWriter writer(packet.data.data(), packet.data.size());

    // construct packet
    PacketHeader header;
    header.sequence = 0;
    header.flags = kConnectResponse;
    header.checksum = 0;
    header.netId = clientNetId_;
    header.time = 0;
    header.size = sizeof(uint64_t);
    header.iteration = iteration_;

    writer.writeRaw(&header, sizeof(header));
    writer.writeLong(cookie_);

    // calc checksum and rewrite header
    header.checksum = checksumPacket(packet, clientXorGen_);

    writer.seek(0);
    writer.writeRaw(&header, sizeof(header));

    // send packet
    manager_.send(packet);

    LOG(Net, Info) << address_ << " sent connect response\n";
}

void Session::handleBlobFragments(BinReader& reader)
{
    // TODO
    (void)reader;
}

void Session::handleServerSwitch(BinReader& reader)
{
    reader.readRaw(8);
    manager_.setPrimary(this);

    LOG(Net, Info) << address_ << " received server switch\n";
}

void Session::handleRequestRetransmit(BinReader& reader)
{
    uint32_t numSequence = reader.readInt();

    for(uint32_t i = 0; i < numSequence; i++)
    {
        uint32_t sequence = reader.readInt();

        auto it = clientPackets_.find(sequence);

        if(it == clientPackets_.end())
        {
            LOG(Net, Warn) << address_ << " ignoring retransmit request of " << sequence << "\n";
            continue;
        }

        manager_.send(*it->second);
    }
}

void Session::handleRejectRetransmit(BinReader& reader)
{
    uint32_t numSequence = reader.readInt();

    for(uint32_t i = 0; i < numSequence; i++)
    {
        uint32_t sequence = reader.readInt();

        if(sequence < serverSequence_)
        {
            // don't care!
            continue;
        }

        serverPackets_.insert(sequence);
    }

    advanceServerSequence();
}

void Session::handleAckSequence(BinReader& reader)
{
    clientSequence_ = max(clientSequence_, reader.readInt());

    for(auto it = clientPackets_.begin(); it != clientPackets_.end(); /**/)
    {
        if(it->first > clientSequence_)
        {
            break;
        }

        it = clientPackets_.erase(it);
    }
}

void Session::handleReferral(BinReader& reader)
{
    uint64_t cookie = reader.readLong();
    /*family*/ reader.readShort();
    uint16_t port = htons(reader.readShort());
    uint32_t ip = htonl(reader.readInt());
    /*zero*/ reader.readRaw(8);

    Address referral(ip, port);

    if(manager_.exists(referral))
    {
        LOG(Net, Warn) << address_ << " received referral for existing session " << referral << "\n";
        return;
    }

    LOG(Net, Info) << address_ << " received referral to " << referral << "\n";

    unique_ptr<Session> session(new Session(manager_, referral, cookie));

    manager_.add(move(session));
}

void Session::handleConnect(const PacketHeader* header, BinReader& reader)
{
    double beginTime = reader.readDouble();
    uint64_t cookie = reader.readLong();
    uint16_t clientNetId = static_cast<uint16_t>(reader.readInt());
    uint32_t serverSeed = reader.readInt();
    uint32_t clientSeed = reader.readInt();
    reader.readInt(); // padding

    cookie_ = cookie;
    serverSequence_ = 1;
    serverLeadingSequence_ = 1;
    clientSequence_ = 1;
    clientLeadingSequence_ = 1;
    serverNetId_ = header->netId;
    clientNetId_ = clientNetId;
    iteration_ = header->iteration;

    serverXorGen_.init(serverSeed);
    clientXorGen_.init(clientSeed);

    beginTime_ = beginTime;
    beginLocalTime_ = net_clock::now();

    serverPackets_.clear();
    clientPackets_.clear();

    LOG(Net, Info) << address_ << " received connect\n";
}

void Session::handleTimeSync(BinReader& reader)
{
    beginTime_ = reader.readDouble();
    beginLocalTime_ = net_clock::now();
}

void Session::handleEchoResponse(BinReader& reader)
{
    numPeriodicSent_ = 0;
    // Ignored for now
    /*pingTime*/ reader.readFloat();
    /*pingResult*/ reader.readFloat();
}

void Session::handleFlow(BinReader& reader)
{
    // Ignored for now
    /*numBytes*/ reader.readInt();
    /*time*/ reader.readShort();
}

void Session::advanceServerSequence()
{
    auto it = serverPackets_.begin();

    while(it != serverPackets_.end())
    {
        if(*it != serverSequence_ + 1)
        {
            break;
        }

        it = serverPackets_.erase(it);
        serverSequence_++;
    }

    serverXorGen_.purge(serverSequence_);
}
