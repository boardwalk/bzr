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
#include "net/Socket.h"
#include "BinReader.h"
#include "BinWriter.h"

enum OptionalHeaderFlags
{
    kDisposable      = 0x00000001,
    kExclusive       = 0x00000002,
    kNotConn         = 0x00000004,
    kTimeSensitive   = 0x00000008,
    kShouldPiggyBack = 0x00000010,
    kHighPriority    = 0x00000020,
    kCountsAsTouch   = 0x00000040,
    kEncrypted       = 0x20000000,
    kSigned          = 0x40000000
};

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

Session::Session(uint32_t serverIp, uint16_t serverPort) : serverIp_(serverIp), serverPort_(serverPort)
{}

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
        handleServerSwitch(header, reader);
    }
    else if(flags == kReferral)
    {
        handleReferral(header, reader);
    }
    else if(flags == kConnectRequest)
    {
        handleConnect(header, reader);
    }
    else
    {
        if(flags & kRequestRetransmit)
        {
            handleRequestRetransmit(header, reader);
            flags & ~kRequestRetransmit;
        }

        if(flags & kRejectRetransmit)
        {
            handleRejectRetransmit(header, reader);
            flags & ~kRejectRetransmit;
        }

        if(flags & kAckSequence)
        {
            handleAckSequence(header, reader);
            flags & ~kAckSequence;
        }

        if(flags & kTimeSync)
        {
            handleTimeSync(header, reader);
            flags & ~kTimeSync;
        }

        if(flags & kEchoResponse)
        {
            handleEchoResponse(header, reader);
            flags & ~kEchoResponse;
        }

        if(flags & kBlobFragments)
        {
            handleBlobFragments(header, reader)
            flags & ~kBlobFragments;
        }

        if(flags != 0)
        {
            throw runtime_error("extra flags in packet header");
        }
    }
}

void Session::tick(net_time_point /*now*/)
{}

uint32_t Session::serverIp() const
{
    return serverIp_;
}

uint16_t Session::remotePort() const
{
    return serverPort_ + (connected_ ? 1 : 0);
}

bool Session::dead() const
{
    return false;
}

net_time_point Session::nextTick() const
{
    return net_time_point::max();
}

void Session::sendLogon(const string& name, const void* key, size_t keyLen)
{
    Packet packet;
    BinWriter writer(packet.data.data(), packet.data.size());

    size_t headerOff = writer.position();
    writer.skip(sizeof(PacketHeader));
    writer.writeString("1802"); // NetVersion
    size_t authDataLenOff = writer.position();
    writer.skip(sizeof(uint32_t));
    writer.writeInt(0x40000002); // GLSUserNameTicket_NetAuthType
    writer.writeInt(0);
    writer.writeInt(static_cast<uint32_t>(time(NULL)));
    writer.writeString(name);
    writer.writeInt(0);
    writer.writeInt(static_cast<uint32_t>(keyLen));
    writer.writeRaw(key, keyLen);

    packet.size = writer.position();

    /// fill in auth data len
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

    send(packet);
}

void Session::sendConnectResponse(uint64_t cookie)
{
    Packet packet;
    BinWriter writer(packet.data.data(), packet.data.size());

    PacketHeader header;
    header.sequence = 0;
    header.flags = kConnectResponse;
    header.checksum = 0;
    header.netId = clientNetId_;
    header.time = 0;
    header.size = sizeof(uint64_t);
    header.iteration = iteration_;

    size_t headerOff = writer.position();
    writer.writeRaw(&header, sizeof(header));
    writer.writeLong(cookie);

    // calc checksum and rewrite header
    header.checksum = checksumPacket(packet, clientXorGen_);

    writer.seek(headerOff);
    writer.writeRaw(&header, sizeof(header));

    send(packet);
}

void Session::send(const Packet& /*packet*/)
{}

void Session::handleServerSwitch(const PacketHeader* header, BinReader& reader)
{
    // TODO make ourselves the primary session in the session manager
}

void Session::handleReferral(const PacketHeader* header, BinReader& reader)
{
    // TODO add a session to the session manager and send a referred packet with the token
}

void Session::handleConnect(const PacketHeader* header, BinReader& reader)
{
    serverSequence_ = 2;
    clientSequence_ = 2;
    serverNetId_ = header->netId;
    iteration_ = header->iteration;

    beginTime_ = reader.readDouble();
    uint16_t cookie = reader.readLong();
    clientNetId_ = static_cast<uint16_t>(header->readInt());
    serverXorGen_.init(reader.readInt());
    clientXorGen_.init(reader.readInt());

    reader.readInt(); // padding

    assert(reader.remaining() == 0);

    serverPort_++;

    // TODO we need to send this multiple times
    // Looks like one every 0.3 seconds
    // How many times?
    sendConnectReply(cookie);
}
