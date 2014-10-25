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
#include "Core.h"
#include "Log.h"

BinReader::BinReader(const void* data, size_t size) : data_{data}, size_{size}, position_{0}
{}

const uint8_t* BinReader::readRaw(size_t size)
{
    assert(size <= remaining());

    const uint8_t* ptr = reinterpret_cast<const uint8_t*>(data_) + position_;

    position_ += size;

    return ptr;
}

uint8_t BinReader::readByte()
{
    return *readPointer<uint8_t>();
}

uint16_t BinReader::readShort()
{
    return *readPointer<uint16_t>();
}

uint32_t BinReader::readInt()
{
    return *readPointer<uint32_t>();
}

uint64_t BinReader::readLong()
{
    return *readPointer<uint64_t>();
}

float BinReader::readFloat()
{
    return *readPointer<float>();
}

double BinReader::readDouble()
{
    return *readPointer<double>();
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

uint32_t BinReader::readPackedInt()
{
    uint32_t val = readShort();

    if(val == 0xFFFF)
    {
        val = readInt();
    }

    return val;
}

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
    position_ = (position_ + 3) & ~3;
}

void BinReader::dump(size_t maxLen) const
{
    const size_t kBytesPerLine = 32;

    string hexPart;
    string asciiPart;

    for(size_t i = 0; i < maxLen && position_ + i < size_; i++)
    {
        if(i != 0 && i % kBytesPerLine == 0)
        {
            LOG(Misc, Debug) << hexPart << " " << asciiPart << "\n";
            hexPart.clear();
            asciiPart.clear();
        }

        uint8_t val = reinterpret_cast<const uint8_t*>(data_)[position_ + i];

        char buf[4];
        sprintf(buf, "%02x ", val);
        hexPart += buf;
        asciiPart += (isprint(val) ? val : '.');
    }

    if(!asciiPart.empty())
    {
        while(asciiPart.size() < kBytesPerLine)
        {
            hexPart += "   ";
            asciiPart += ' ';
        }

        LOG(Misc, Debug) << hexPart << " " << asciiPart << "\n";
    }
}

size_t BinReader::position() const
{
    return position_;
}

size_t BinReader::remaining() const
{
    return size_ - position_;
}
