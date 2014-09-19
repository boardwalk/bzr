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
    BinReader(const void* data, size_t size);

    template<class T>
    const T& read();

    template<class T>
    const T* readPointer(size_t count = 1);

    uint16_t readPackedShort();

    string readString();

    void align();

    void assertEnd() const;

private:
    void assertRemaining(size_t numBytes) const;

    const uint8_t* data_;
    size_t size_;
    size_t position_;
};

template<class T>
const T& BinReader::read()
{
    assertRemaining(sizeof(T));

    const T* result = reinterpret_cast<const T*>(data_ + position_);

    position_ += sizeof(T);

    return *result;
}

template<class T>
const T* BinReader::readPointer(size_t count)
{
    assertRemaining(sizeof(T) * count);

    const T* result = reinterpret_cast<const T*>(data_ + position_);

    position_ += sizeof(T) * count;

    return result;
}

#endif
