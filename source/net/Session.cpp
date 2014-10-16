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
#include "BinWriter.h"

enum PacketFlags
{
    kLogon = 0x00010000
};

PACK(struct PacketHeader {
    uint32_t sequence;
    uint32_t flags;
    uint32_t checksum;
    uint16_t endpoint;
    uint16_t time;
    uint16_t size;
    uint16_t session;
});

void Session::handle(const Packet&)
{}

void Session::tick(net_time_point /*now*/)
{}

uint32_t Session::remoteIp() const
{
    return 0;
}

uint16_t Session::remotePort() const
{
    return 0;
}

bool Session::dead() const
{
    return false;
}

net_time_point Session::nextTick() const
{
    return net_time_point::max();
}

void Session::sendLogon(const string& name, const string& key)
{
    Packet packet;

    BinWriter writer(packet.data.data(), packet.data.size());

    writer.skip(sizeof(PacketHeader));

    PacketHeader header;
    header.sequence = 0;
    header.flags = PacketFlags::kLogon;
    //header.checksum
    header.endpoint = 0;
    header.time = 0;
    header.size = packet.data.size() - writer.remaining();
    header.session = 0;

    writer.seek(0);
    writer.writeRaw(&header, sizeof(header));
}
