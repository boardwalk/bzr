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

#include "net/ChecksumXorGenerator.h"
#include <chrono>
#include <map>

typedef chrono::high_resolution_clock net_clock;
typedef net_clock::time_point net_time_point;

class BinReader;
struct Packet;
struct PacketHeader;

class Session
{
public:
    Session(SessionManager& manager,
        uint32_t serverIp,
        uint16_t serverPort,
        const string& accountName,
        const string& accountKey);
    Session(SessionManager& manager,
        uint32_t serverIp,
        uint16_t serverPort,
        uint64_t cookie);

    void handle(const Packet& packet);
    void tick(net_time_point now);

    uint32_t serverIp() const;
    uint16_t serverPort() const;
    bool dead() const;
    net_time_point nextTick() const;
    
private:
    void sendLogon(const string& name, const void* key, size_t keyLen);
    void sendConnectResponse(uint64_t cookie);
    void send(const Packet& packet);

    void handleBlobFragments(const PacketHeader* header, BinReader& reader);
    void handleServerSwitch(const PacketHeader* header, BinReader& reader);
    void handleRequestRetransmit(const PacketHeader* header, BinReader& reader);
    void handleRejectRetransmit(const PacketHeader* header, BinReader& reader);
    void handleAckSequence(const PacketHeader* header, BinReader& reader);
    void handleReferral(const PacketHeader* header, BinReader& reader);    
    void handleConnect(const PacketHeader* header, BinReader& reader);
    void handleTimeSync(const PacketHeader* header, BinReader& reader);
    void handleEchoResponse(const PacketHeader* header, BinReader& reader);
    void handleFlow(const PacketHeader* header, BinReader& reader);

    void advanceServerSequence();
    
    enum class State
    {
        kLogon,
        kReferred,
        kConnectResponse,
        kConnected
    };

    SessionManager& manager_;
    const uint32_t serverIp_;
    const uint16_t serverPort_;
    State state_;

    // for State::kLogon
    string accountName_;
    string accountKey_;

    // for State::kReferred and State::kConnect
    uint64_t cookie_;

    net_time_point nextPeriodic_;

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