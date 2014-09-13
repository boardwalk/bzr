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
    BinReader(const void* data, size_t size) : _data(data), _size(size), _position(0)
    {}

    template<class T>
    T read()
    {
        assertRemaining(sizeof(T));

        auto result = *(const T*)((const uint8_t*)_data + _position);

        _position += sizeof(T);

        return result;
    }

    template<class T>
    const T* readPointer(size_t count = 1)
    {
        assertRemaining(sizeof(T) * count);

        auto result = (const T*)((const uint8_t*)_data + _position);

        _position += sizeof(T) * count;

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

        auto data = readPointer<char>(count);

        align();

        return string(data, data + count);
    }

    void align()
    {
        while(_position & 3)
        {
            _position++;
        }
    }

    void assertEnd() const
    {
        if(_position < _size)
        {
           throw runtime_error("Expected end of blob");
        }
    }

private:
    void assertRemaining(size_t numBytes) const
    {
        if(_position + numBytes > _size)
        {
            throw runtime_error("Read overrun in blob");
        }
    }

    const void* _data;
    size_t _size;
    size_t _position;
};

#endif
