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
#include "BinReader.h"

template<class T>
T readPrimitive(BinReader& reader)
{
    return *reinterpret_cast<const T*>(reader.readRaw(sizeof(T)));
}

BinReader::BinReader(const void* data, size_t size) : data_{data}, size_{size}, position_{0}
{}

const uint8_t* BinReader::readRaw(size_t size)
{
    if(position_ + size > size_)
    {
        throw runtime_error("Read overrun");
    }

    const uint8_t* ptr = reinterpret_cast<const uint8_t*>(data_) + position_;

    position_ += size;

    return ptr;
}

uint8_t BinReader::readByte()
{
    return readPrimitive<uint8_t>(*this);
}

uint16_t BinReader::readShort()
{
    return readPrimitive<uint16_t>(*this);
}

uint32_t BinReader::readInt()
{
    return readPrimitive<uint32_t>(*this);
}

uint64_t BinReader::readLong()
{
    return readPrimitive<uint64_t>(*this);
}

float BinReader::readFloat()
{
    return readPrimitive<float>(*this);
}

double BinReader::readDouble()
{
    return readPrimitive<double>(*this);
}

uint16_t BinReader::readPackedShort()
{
    uint16_t val = readByte();

    if(val & 0x80)
    {
        val = (val & 0x7F) << 8 | readByte();
    }

    return val;
}

// TODO readPackedInt

string BinReader::readString()
{
    uint32_t count = readShort();

    if(count == 0xFFFF)
    {
        count = readInt();
    }

    const uint8_t* data = readRaw(count);

    align();

    return string(data, data + count);
}

void BinReader::align()
{
    position_ += 3 - ((position_ - 1) & 3);
}

void BinReader::assertEnd() const
{
    if(position_ < size_)
    {
       throw runtime_error("Expected end of blob");
    }
}
