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
#ifndef NET_ADDRESS_H
#define NET_ADDRESS_H

class Address
{
public:
    Address();
    Address(uint32_t ip, uint16_t port);

    uint32_t ip() const;
    uint16_t port() const;

private:
    uint32_t ip_;
    uint16_t port_;
};

bool operator==(Address a, Address b);
bool operator!=(Address a, Address b);

ostream& operator<<(ostream& os, Address addr);

#endif