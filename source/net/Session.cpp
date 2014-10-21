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

static chrono::milliseconds kLogonPacketDelay(300);
static chrono::milliseconds kReferredPacketDelay(300);
static chrono::milliseconds kConnectResponsePacketDelay(300);

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
                const BlobHeader* blobHeader = reinterpret_cast<const BlobHeader*>(reader.readRaw(sizeof(BlobHeader)));

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
    uint32_t serverIp,
    uint16_t serverPort,
    const string& accountName,
    const string& accountKey) :
    manager_(manager),
    serverIp_(serverIp),
    serverPort_(serverPort),
    state_(State::kLogon),
    accountName_(accountName),
    accountKey_(accountKey)
{
    sendLogon();
}

Session::Session(SessionManager& manager,
    uint32_t serverIp,
    uint16_t serverPort,
    uint64_t cookie) :
    manager_(manager),
    serverIp_(serverIp),
    serverPort_(serverPort),
    state_(State::kReferred),
    cookie_(cookie)
{
    sendReferred();
}

void Session::handle(const Packet& packet)
{
    BinReader reader(packet.data.data(), packet.size);

    const PacketHeader* header = reinterpret_cast<const PacketHeader*>(reader.readRaw(sizeof(PacketHeader)));

    if(header->size != reader.remaining())
    {
        // TODO proper logging
        fprintf(stderr, "WARNING: bad size in packet header\n");
        return;
    }

    const uint32_t calcChecksum = checksumPacket(packet, serverXorGen_);

    if(header->checksum != calcChecksum)
    {
        // TODO proper logging
        fprintf(stderr, "WARNING: bad checksum in packet header\n");
        return;
    }

    uint32_t flags = header->flags;
    flags &= ~(kRetransmission | kEncryptedChecksum);

    if(flags == kServerSwitch)
    {
        handleServerSwitch(reader);
    }
    else if(flags == kReferral)
    {
        handleReferral(reader);
    }
    else if(flags == kConnectRequest)
    {
        handleConnect(header, reader);
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
            throw runtime_error("extra flags in packet header");
        }
    }

    assert(reader.remaining() == 0);
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

uint32_t Session::serverIp() const
{
    return serverIp_;
}

uint16_t Session::serverPort() const
{
    return serverPort_ + (state_ == State::kConnectResponse || state_ == State::kConnected) ? 1 : 0;
}

bool Session::dead() const
{
    return false;
}

net_time_point Session::nextTick() const
{
    return nextPeriodic_;
}

void Session::sendLogon()
{
    assert(state_ == State::kLogon);

    Packet packet;
    BinWriter writer(packet.data.data(), packet.data.size());

    // construct packet
    size_t headerOff = writer.position();
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
    memset(&header, 0, sizeof(header));
    header.flags = PacketFlags::kLogon;
    header.size = static_cast<uint16_t>(packet.size - sizeof(PacketHeader));

    writer.seek(headerOff);
    writer.writeRaw(&header, sizeof(header));

    // calculate checksum and rewrite header
    header.checksum = checksumPacket(packet, clientXorGen_);

    writer.seek(headerOff);
    writer.writeRaw(&header, sizeof(header));

    // send packet
    manager_.send(packet);
    nextPeriodic_ = net_clock::now() + kLogonPacketDelay;
}

void Session::sendReferred()
{
    assert(state_ == State::kReferred);

    Packet packet;
    BinWriter writer(packet.data.data(), packet.data.size());

    // construct packet
    PacketHeader header;
    memset(&header, 0, sizeof(header));
    header.flags = kReferred;
    header.size = sizeof(uint64_t);

    writer.writeRaw(&header, sizeof(header));
    writer.writeLong(cookie_);

    // calc checksum and rewrite header
    header.checksum = checksumPacket(packet, clientXorGen_);

    writer.seek(0);
    writer.writeRaw(&header, sizeof(header));

    // send packet
    manager_.send(packet);
    nextPeriodic_ = net_clock::now() + kReferredPacketDelay;
}

void Session::sendConnectResponse()
{
    assert(state_ == State::kConnectResponse);

    Packet packet;
    BinWriter writer(packet.data.data(), packet.data.size());

    // construct packet
    PacketHeader header;
    memset(&header, 0, sizeof(header));
    header.flags = kConnectResponse;
    header.netId = clientNetId_;
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
    nextPeriodic_ = net_clock::now() + kConnectResponsePacketDelay;
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
            throw runtime_error("Server requested packet that does not exist");
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

        serverPackets_[sequence] = true;
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
    uint16_t serverPort = htons(reader.readShort());
    uint32_t serverIp = htonl(reader.readInt());
    /*zero*/ reader.readRaw(8);

    if(manager_.exists(serverIp, serverPort))
    {
        return;
    }

    unique_ptr<Session> session(new Session(
        manager_,
        serverIp,
        serverPort,
        cookie));

    manager_.add(move(session));
}

void Session::handleConnect(const PacketHeader* header, BinReader& reader)
{
    if(state_ != State::kLogon && state_ != State::kReferred)
    {
        throw runtime_error("Connect received in wrong state");
    }

    double beginTime = reader.readDouble();
    uint64_t cookie = reader.readLong();
    uint16_t clientNetId = static_cast<uint16_t>(reader.readInt());
    uint32_t serverSeed = reader.readInt();
    uint32_t clientSeed = reader.readInt();
    reader.readInt(); // padding

    state_ = State::kConnectResponse;
    cookie_ = cookie;
    serverSequence_ = 2;
    clientSequence_ = 2;
    clientLeadingSequence_ = 2;
    serverNetId_ = header->netId;
    clientNetId_ = clientNetId;
    iteration_ = header->iteration;

    serverXorGen_.init(serverSeed);
    clientXorGen_.init(clientSeed);

    beginTime_ = beginTime;
    beginLocalTime_ = net_clock::now();

    serverPackets_.clear();
    clientPackets_.clear();

    sendConnectResponse();
}

void Session::handleTimeSync(BinReader& reader)
{
    beginTime_ = reader.readDouble();
    beginLocalTime_ = net_clock::now();
}

void Session::handleEchoResponse(BinReader& reader)
{
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
        if(it->first != serverSequence_)
        {
            break;
        }

        it = serverPackets_.erase(it);
        serverSequence_++;
    }
}
