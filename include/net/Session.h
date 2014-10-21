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
#include "net/ChecksumXorGenerator.h"
#include "Noncopyable.h"
#include <chrono>
#include <map>

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
        const string& accountName,
        const string& accountKey);
    Session(SessionManager& manager,
        Address address,
        uint64_t cookie);

    void handle(const Packet& packet);
    void tick(net_time_point now);

    Address address() const;
    bool dead() const;
    net_time_point nextTick() const;

private:
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
    void handleConnect(const PacketHeader* header, BinReader& reader);
    void handleTimeSync(BinReader& reader);
    void handleEchoResponse(BinReader& reader);
    void handleFlow(BinReader& reader);

    void advanceServerSequence();

    SessionManager& manager_;
    const Address address_;
    State state_;

    // for State::kLogon
    string accountName_;
    string accountKey_;

    // for State::kReferred and State::kConnect
    uint64_t cookie_;

    net_time_point nextPeriodic_;
    size_t numPeriodicSent_;

    uint32_t serverSequence_;
    uint32_t clientSequence_;
    uint32_t clientLeadingSequence_;
    uint16_t serverNetId_;
    uint16_t clientNetId_;
    uint16_t iteration_;

    ChecksumXorGenerator serverXorGen_;
    ChecksumXorGenerator clientXorGen_;

    double beginTime_;
    net_time_point beginLocalTime_;

    map<uint32_t, bool> serverPackets_;
    map<uint32_t, unique_ptr<Packet>> clientPackets_;
};

#endif