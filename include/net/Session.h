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
#ifndef BZR_NET_SESSION_H
#define BZR_NET_SESSION_H

#include "net/Address.h"
#include "net/BlobAssembler.h"
#include "net/ChecksumXorGenerator.h"
#include "Noncopyable.h"
#include <chrono>
#include <map>
#include <set>

typedef chrono::high_resolution_clock net_clock;
typedef net_clock::time_point net_time_point;

class BinReader;
struct Packet;
struct PacketHeader;
class SessionManager;

class Session : Noncopyable
{
public:
    Session(SessionManager& manager,
        Address address,
        string accountName,
        string accountKey);
    Session(SessionManager& manager,
        Address address,
        uint64_t cookie);
    ~Session();

    void handle(const Packet& packet);
    void tick(net_time_point now);

    Address address() const;
    bool dead() const;
    net_time_point nextTick() const;
    BlobAssembler& blobAssembler();

private:
    typedef chrono::duration<uint16_t, ratio<1, 2>> SessionDuration;

    enum class State
    {
        kLogon,
        kReferred,
        kConnectResponse,
        kConnected
    };

    void sendLogon();
    void sendReferred();
    void sendConnectResponse();

    void handleBlobFragments(BinReader& reader);
    void handleServerSwitch(BinReader& reader);
    void handleRequestRetransmit(BinReader& reader);
    void handleRejectRetransmit(BinReader& reader);
    void handleAckSequence(BinReader& reader);
    void handleReferral(BinReader& reader);
    void handleConnect(BinReader& reader, const PacketHeader& header);
    void handleTimeSync(BinReader& reader);
    void handleEchoResponse(BinReader& reader);
    void handleFlow(BinReader& reader);

    void advanceServerSequence();

    SessionManager& manager_;
    const Address address_;
    State state_;

    // the time at which this session was created
    net_time_point sessionBegin_;

    // the time at which the next logon, referred, connect response or ping packet should be sent
    net_time_point nextPeriodic_;

    // the number of times the periodic packet for this state has been sent without a response
    size_t numPeriodicSent_;

    //
    // State::kLogon
    //
    string accountName_;
    string accountKey_;

    //
    // State::kReferred, State::kConnectResponse
    //
    uint64_t cookie_;

    //
    // State::kConnectResponse, State::kConnected
    //

    // the latest server sequence acknowledged by the client
    uint32_t serverSequence_;

    // the latest contiguous sequence received by the client, not necessarily acknowledged yet
    // serverLeadingSequence_ >= serverSequence_
    uint32_t serverLeadingSequence_;

    // the latest client sequence acknowledged by the server
    uint32_t clientSequence_;

    // the latest contiguous sequence sent by the client, not necessarily acknowledged yet
    // clientLeadingSequence_ >= clientSequence_
    uint32_t clientLeadingSequence_;

    // the value the server should use in the packet header's net id field
    uint16_t serverNetId_;

    // the value the client should use in the packet header's net id field
    uint16_t clientNetId_;

    // the value both the server and client should use in the packet header's iteration field
    uint16_t iteration_;

    // the rng used to generate xor values for server packets
    ChecksumXorGenerator serverXorGen_;

    // the rng used to generate xor values for client packets
    ChecksumXorGenerator clientXorGen_;

    // the time specified in the last connect packet or time sync header
    double beginTime_;

    // the local time at which beginTime_ was set
    net_time_point beginLocalTime_;

    // stores the set of all sequences > serverSequence_ sent by the server
    set<uint32_t> serverPackets_;

    // stores all packets with sequence > clientSequence_ sent by the client
    map<uint32_t, unique_ptr<Packet>> clientPackets_;

    net_time_point nextRequestMissing_;

    uint16_t lastFlowTime_;
    uint32_t lastFlowBytes_;

    BlobAssembler blobAssembler_;
};

#endif