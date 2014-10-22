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
#ifndef BZR_NET_PACKET_H
#define BZR_NET_PACKET_H

#include "net/Address.h"

const size_t kPayloadMaxSize = 464;

PACK(struct PacketHeader {
    uint32_t sequence;
    uint32_t flags;
    uint32_t checksum;
    uint16_t netId;
    uint16_t time;
    uint16_t size;
    uint16_t iteration;
});

struct Packet
{
    Address address;
    PacketHeader header;
    uint8_t payload[kPayloadMaxSize];
};

static_assert(offsetof(Packet, header) + sizeof(PacketHeader) == offsetof(Packet, payload),
        "Padding between header and payload");

#endif
