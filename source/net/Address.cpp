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
#include "net/Address.h"

Address::Address() : ip_(0), port_(0)
{}

Address::Address(uint32_t ip, uint16_t port) : ip_(ip), port_(port)
{}

uint32_t Address::ip() const
{
    return ip_;
}

uint16_t Address::port() const
{
    return port_;
}

bool operator==(Address a, Address b)
{
    return a.ip() == b.ip() && a.port() == b.port();
}

bool operator!=(Address a, Address b)
{
    return a.ip() != b.ip() || a.port() != b.port();
}

ostream& operator<<(ostream& os, Address addr)
{
    return os << (addr.ip() >> 24) << '.'
        << ((addr.ip() >> 16) & 0xFF) << '.'
        << ((addr.ip() >> 8) & 0xFF) << '.'
        << (addr.ip() & 0xFF) << ':'
        << addr.port();
}
