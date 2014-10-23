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
#include <algorithm>
#ifndef _WIN32
#include <arpa/inet.h>
#endif

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

static const chrono::milliseconds kLogonPacketDelay(300);
static const chrono::milliseconds kReferredPacketDelay(300);
static const chrono::milliseconds kConnectResponsePacketDelay(300);
static const chrono::milliseconds kPingPacketDelay(2000);
static const chrono::milliseconds kMissingPacketDelay(300);
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

static uint32_t checksumHeader(const PacketHeader& header)
{
    PacketHeader& header_nc = const_cast<PacketHeader&>(header);

    uint32_t origChecksum = header.checksum;
    header_nc.checksum = 0xBADD70DD;

    uint32_t result = checksum(&header, sizeof(PacketHeader));

    header_nc.checksum = origChecksum;

    return result;
}

static uint32_t checksumContent(const PacketHeader& header, const void* data)
{
    if(header.flags & kEncryptedChecksum)
    {
        BinReader reader(data, header.size);

        if(header.flags & kServerSwitch)
        {
            reader.readRaw(8);
        }

        if(header.flags & kRequestRetransmit)
        {
            uint32_t nseq = reader.readInt();
            reader.readRaw(nseq * sizeof(uint32_t));
        }

        if(header.flags & kRejectRetransmit)
        {
            uint32_t nseq = reader.readInt();
            reader.readRaw(nseq * sizeof(uint32_t));
        }

        if(header.flags & kCICMDCommand)
        {
            reader.readRaw(8);
        }

        if(header.flags & kTimeSync)
        {
            reader.readRaw(8);
        }

        if(header.flags & kEchoRequest)
        {
            reader.readRaw(4);
        }

        if(header.flags & kEchoResponse)
        {
            reader.readRaw(8);
        }

        if(header.flags & kFlow)
        {
            reader.readRaw(6);
        }

        uint32_t result = checksum(data, reader.position());

        if(header.flags & kBlobFragments)
        {
            while(reader.remaining() != 0)
            {
                const FragmentHeader* fragment = reader.readPointer<FragmentHeader>();

                reader.readRaw(fragment->size - sizeof(FragmentHeader));

                result += checksum(fragment, fragment->size);
            }
        }

        assert(reader.remaining() == 0);

        return result;
    }

    return checksum(data, header.size);
}

static uint32_t checksumPacket(const Packet& packet, ChecksumXorGenerator& xorGen)
{
    uint32_t xorVal = (packet.header.flags & kEncryptedChecksum) ? xorGen.get(packet.header.sequence) : 0;
    return checksumHeader(packet.header) + (checksumContent(packet.header, packet.payload) ^ xorVal);
}

Session::Session(SessionManager& manager,
    Address address,
    string accountName,
    string accountKey) :
    manager_(manager),
    address_(address),
    state_(State::kLogon),
    sessionBegin_(net_clock::now()),
    numPeriodicSent_(0),
    accountName_(move(accountName)),
    accountKey_(move(accountKey))
{
    LOG(Net, Info) << address_ << " logon session created\n";
    sendLogon();
}

Session::Session(SessionManager& manager,
    Address address,
    uint64_t cookie) :
    manager_(manager),
    address_(address),
    state_(State::kReferred),
    sessionBegin_(net_clock::now()),
    numPeriodicSent_(0),
    cookie_(cookie)
{
    LOG(Net, Info) << address_ << " referred session created\n";
    sendReferred();
}

Session::~Session()
{
    LOG(Net, Info) << address_ << " destroyed\n";
}

void Session::handle(const Packet& packet)
{
    uint32_t calcChecksum = checksumPacket(packet, serverXorGen_);

    if(packet.header.checksum != calcChecksum)
    {
        LOG(Net, Warn) << address_ << " dropping packet, calc checksum is " << hexn(calcChecksum) << ", packet has " << hexn(packet.header.checksum) << "\n";
        return;
    }

    BinReader reader(packet.payload, sizeof(packet.payload));

    if(state_ == State::kConnectResponse)
    {
        state_ = State::kConnected;
        nextPeriodic_ = net_clock::now() + kPingPacketDelay;
        numPeriodicSent_ = 0;

        LOG(Net, Info) << address_ << " transitioned to connected\n";
    }

    if(state_ == State::kLogon || state_ == State::kReferred)
    {
        if(packet.header.flags != kConnectRequest)
        {
            LOG(Net, Warn) << address_ << "dropping packet, flags should be " << hexn(static_cast<uint32_t>(kConnectRequest)) << ", packet has " << hexn(packet.header.flags) << "\n";
            return;
        }

        handleConnect(reader, packet.header);

        state_ = State::kConnectResponse;
        numPeriodicSent_ = 0;
        sendConnectResponse();
    }
    else if(state_ == State::kConnected)
    {
        if(packet.header.sequence <= serverSequence_ || serverPackets_.find(packet.header.sequence) != serverPackets_.end())
        {
            LOG(Net, Warn) << address_ << " dropping packet, already received sequence " << packet.header.sequence << "\n";
            return;
        }

        if(packet.header.netId != serverNetId_)
        {
            LOG(Net, Warn) << address_ << " dropping packet, server net id is " << serverNetId_ << ", packet has " << packet.header.netId << "\n";
            return;
        }

        if(packet.header.iteration != iteration_)
        {
            LOG(Net, Warn) << address_ << " dropping packet, iteration is " << iteration_ << ", packet has " << packet.header.iteration << "\n";
            return;
        }

        // record that we've received the packet
        serverPackets_.insert(packet.header.sequence);
        advanceServerSequence();

        // record that we've received some bytes
        lastFlowBytes_ += static_cast<uint32_t>(sizeof(PacketHeader) + packet.header.size);

        uint32_t flags = packet.header.flags & ~(kRetransmission | kEncryptedChecksum);

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
    else
    {
        throw logic_error("bad state");
    }

    if(reader.remaining() != 0)
    {
        LOG(Net, Error) << address_ << " unconsumed data of " << reader.remaining() << " bytes in packet\n";
        throw runtime_error("unconsumed data");
    }
}

void Session::tick(net_time_point now)
{
    if(state_ == State::kLogon)
    {
        if(now > nextPeriodic_)
        {
            sendLogon();
        }
    }
    else if(state_ == State::kReferred)
    {
        if(now > nextPeriodic_)
        {
            sendReferred();
        }
    }
    else if(state_ == State::kConnectResponse)
    {
        if(now > nextPeriodic_)
        {
            sendConnectResponse();
        }
    }
    else if(state_ == State::kConnected)
    {
        Packet packet;

        BinWriter writer(packet.payload, sizeof(packet.payload));

        uint16_t time = chrono::duration_cast<SessionDuration>(now - sessionBegin_).count();
        uint32_t flags = 0;

        if(now > nextRequestMissing_)
        {
            flags |= kRequestRetransmit;
            writer.writeInt(static_cast<uint32_t>(serverPackets_.size()));

            for(uint32_t sequence : serverPackets_)
            {
                writer.writeInt(sequence);
            }

            nextRequestMissing_ = now + kMissingPacketDelay;
        }

        if(serverLeadingSequence_ > serverSequence_)
        {
            flags |= kAckSequence;
            writer.writeInt(serverLeadingSequence_);

            serverSequence_ = serverLeadingSequence_;
        }

        if(now > nextPeriodic_)
        {
            flags |= kTimeSync;
            writer.writeDouble(beginTime_ + chrono::duration<double>(now - beginLocalTime_).count());

            flags |= kEchoRequest;
            writer.writeFloat(chrono::duration<float>(now - manager_.getClientBegin()).count());

            flags |= kFlow;
            writer.writeInt(lastFlowBytes_);
            writer.writeShort(lastFlowTime_);

            lastFlowBytes_ = 0;
            lastFlowTime_ = time;
            nextPeriodic_ = now + kPingPacketDelay;
        }

        if(flags != 0)
        {
            packet.header.sequence = clientLeadingSequence_ + 1;
            packet.header.flags = kEncryptedChecksum;
            packet.header.netId = clientNetId_;
            packet.header.time = time;
            packet.header.size = static_cast<uint16_t>(writer.position());
            packet.header.iteration = iteration_;
            packet.header.checksum = checksumPacket(packet, clientXorGen_); // must be done last
            manager_.send(packet);

            packet.header.flags = flags | kRetransmission;
            packet.header.checksum = checksumPacket(packet, clientXorGen_); // must be done last
            clientPackets_[packet.header.sequence].reset(new Packet(packet));

            clientLeadingSequence_++;
        }
    }
    else
    {
        throw logic_error("bad state");
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
    return min(nextPeriodic_, nextRequestMissing_);
}

BlobAssembler& Session::blobAssembler()
{
    return blobAssembler_;
}

void Session::sendLogon()
{
    Packet packet;

    memset(&packet.header, 0, sizeof(packet.header));
    packet.header.flags = kLogon;

    BinWriter writer(packet.payload, sizeof(packet.payload));
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

    writer.seek(authDataLenOff);
    writer.writeInt(static_cast<uint32_t>(writer.position() - authDataLenOff));

    packet.header.size = static_cast<uint16_t>(writer.position());
    packet.header.checksum = checksumPacket(packet, clientXorGen_); // must be done last
    manager_.send(packet);

    nextPeriodic_ = net_clock::now() + kLogonPacketDelay;
    numPeriodicSent_++;

    LOG(Net, Info) << address_ << " sent logon\n";
}

void Session::sendReferred()
{
    Packet packet;

    memset(&packet.header, 0, sizeof(packet.header));
    packet.header.flags = kReferred;
    packet.header.size = sizeof(uint64_t);

    BinWriter writer(packet.payload, sizeof(packet.payload));
    writer.writeLong(cookie_);

    packet.header.checksum = checksumPacket(packet, clientXorGen_); // must be done last
    manager_.send(packet);

    nextPeriodic_ = net_clock::now() + kReferredPacketDelay;
    numPeriodicSent_++;

    LOG(Net, Info) << address_ << " sent referred\n";
}

void Session::sendConnectResponse()
{
    Packet packet;

    memset(&packet.header, 0, sizeof(packet.header));
    packet.header.flags = kConnectResponse;
    packet.header.netId = clientNetId_;
    packet.header.size = sizeof(uint64_t);
    packet.header.iteration = iteration_;

    BinWriter writer(packet.payload, sizeof(packet.payload));
    writer.writeLong(cookie_);

    packet.header.checksum = checksumPacket(packet, clientXorGen_); // must be done last
    manager_.send(packet);

    nextPeriodic_ = net_clock::now() + kConnectResponsePacketDelay;
    numPeriodicSent_++;

    LOG(Net, Info) << address_ << " sent connect response\n";
}

void Session::handleBlobFragments(BinReader& reader)
{
    while(reader.remaining() != 0)
    {
        const FragmentHeader* fragment = reader.readPointer<FragmentHeader>();

        reader.readRaw(fragment->size - sizeof(FragmentHeader));

        blobAssembler_.addFragment(fragment);
    }
}

void Session::handleServerSwitch(BinReader& reader)
{
    /*sequence*/ reader.readInt();
    /*type*/ reader.readInt();

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

    LOG(Net, Debug) << address_ << " received request retransmit for " << numSequence << " packets\n";
}

void Session::handleRejectRetransmit(BinReader& reader)
{
    uint32_t numSequence = reader.readInt();

    for(uint32_t i = 0; i < numSequence; i++)
    {
        uint32_t sequence = reader.readInt();

        if(sequence > serverSequence_)
        {
            serverPackets_.insert(sequence);
        }
    }

    advanceServerSequence();

    LOG(Net, Debug) << address_ << " received reject retransmit for " << numSequence << " packets\n";
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

    LOG(Net, Debug) << address_ << " received ack sequence " << clientSequence_ << "\n";
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

    unique_ptr<Session> session(new Session(manager_, referral, cookie));

    manager_.add(move(session));

    LOG(Net, Info) << address_ << " received referral to " << referral << "\n";
}

void Session::handleConnect(BinReader& reader, const PacketHeader& header)
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
    serverNetId_ = header.netId;
    clientNetId_ = clientNetId;
    iteration_ = header.iteration;

    serverXorGen_.init(serverSeed);
    clientXorGen_.init(clientSeed);

    beginTime_ = beginTime;
    beginLocalTime_ = net_clock::now();

    serverPackets_.clear();
    clientPackets_.clear();

    nextRequestMissing_ = net_time_point::max();

    lastFlowTime_ = 0;
    lastFlowBytes_ = 0;

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

    if(!serverPackets_.empty())
    {
        if(nextRequestMissing_ == net_time_point::max())
        {
            nextRequestMissing_ = net_clock::now() + kMissingPacketDelay;
        }
    }
    else
    {
        if(nextRequestMissing_ != net_time_point::max())
        {
            nextRequestMissing_ = net_time_point::max();
        }
    }
}
