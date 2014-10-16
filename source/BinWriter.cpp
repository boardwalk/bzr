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
#include "BinWriter.h"

template<class T>
void writePrimitive(BinWriter& writer, T value)
{
    writer.writeRaw(&value, sizeof(value));
}

BinWriter::BinWriter(void* data, size_t size) : data_{data}, size_{size}, position_{0}
{}

void BinWriter::writeRaw(const void* data, size_t size)
{
    assert(size <= remaining());

    uint8_t* ptr = reinterpret_cast<uint8_t*>(data_) + position_;

    memcpy(ptr, data, size);
    position_ += size;
}

void BinWriter::writeByte(uint8_t value)
{
    writePrimitive(*this, value);
}

void BinWriter::writeShort(uint16_t value)
{
    writePrimitive(*this, value);
}

void BinWriter::writeInt(uint32_t value)
{
    writePrimitive(*this, value);
}

void BinWriter::writeLong(uint64_t value)
{
    writePrimitive(*this, value);
}

void BinWriter::writeFloat(float value)
{
    writePrimitive(*this, value);
}

void BinWriter::writeDouble(double value)
{
    writePrimitive(*this, value);
}

void BinWriter::writeString(const string& value)
{
    if(value.size() >= 0xFFFF)
    {
        writeShort(0xFFFF);
        writeInt(static_cast<uint32_t>(value.size()));
    }
    else
    {
        writeShort(static_cast<uint16_t>(value.size()));
    }

    writeRaw(value.data(), value.size());
    align();
}

void BinWriter::skip(size_t amount)
{
    assert(amount <= remaining());

    position_ += amount;
}

void BinWriter::seek(size_t position)
{
    assert(position <= size_);

    position_ = position;
}

void BinWriter::align()
{
    position_ = (position_ + 3) & ~3;
}

size_t BinWriter::position() const
{
    return position_;
}

size_t BinWriter::remaining() const
{
    return size_ - position_;
}
