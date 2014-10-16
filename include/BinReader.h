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
#ifndef BZR_BINREADER_H
#define BZR_BINREADER_H

#include "Noncopyable.h"

class BinReader : Noncopyable
{
public:
    BinReader(const void* data, size_t size);

    const uint8_t* readRaw(size_t size);
    uint8_t readByte();
    uint16_t readShort();
    uint32_t readInt();
    uint64_t readLong();
    float readFloat();
    double readDouble();
    uint16_t readPackedShort();
    uint32_t readPackedInt();
    string readString();

    void align();

    size_t position() const;
    size_t remaining() const;

private:
    const void* data_;
    const size_t size_;
    size_t position_;
};

#endif
