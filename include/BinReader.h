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

class BinReader
{
public:
    BinReader(const void* data, size_t size) : data_(reinterpret_cast<const uint8_t*>(data)), size_(size), position_(0)
    {}

    template<class T>
    T read()
    {
        assertRemaining(sizeof(T));

        const T& result = *reinterpret_cast<const T*>(data_ + position_);

        position_ += sizeof(T);

        return result;
    }

    template<class T>
    const T* readPointer(size_t count = 1)
    {
        assertRemaining(sizeof(T) * count);

        const T* result = reinterpret_cast<const T*>(data_ + position_);

        position_ += sizeof(T) * count;

        return result;
    }

    uint16_t readVarInt()
    {
        uint16_t val = read<uint8_t>();

        if(val & 0x80)
        {
            val = (val & 0x7F) << 8 | read<uint8_t>();
        }

        return val;
    }

    string readString()
    {
        uint32_t count = read<uint16_t>();

        if(count == 0xFFFF)
        {
            count = read<uint32_t>();
        }

        const char* data = readPointer<char>(count);

        align();

        return string(data, data + count);
    }

    void align()
    {
        while(position_ & 3)
        {
            position_++;
        }
    }

    void assertEnd() const
    {
        if(position_ < size_)
        {
           throw runtime_error("Expected end of blob");
        }
    }

private:
    void assertRemaining(size_t numBytes) const
    {
        if(position_ + numBytes > size_)
        {
            throw runtime_error("Read overrun in blob");
        }
    }

    const uint8_t* data_;
    size_t size_;
    size_t position_;
};

#endif
