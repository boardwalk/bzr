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
#ifndef BZR_BINWRITER_H
#define BZR_BINWRITER_H

#include "Noncopyable.h"

class BinWriter : Noncopyable
{
public:
    BinWriter(void* data, size_t size);

    void writeRaw(const void* data, size_t size);
    void writeByte(uint8_t value);
    void writeShort(uint16_t value);
    void writeInt(uint32_t value);
    void writeLong(uint64_t value);
    void writeFloat(float value);
    void writeDouble(double value);
    //void writePackedShort(uint16_t value);
    //void writePackedInt(uint32_t value);
    void writeString(const string& value);

    void skip(size_t amount);
    void seek(size_t position);
    void align();

    size_t position() const;
    size_t remaining() const;

protected:
    BinWriter();

    void* data_;
    size_t size_;
    size_t position_;
};

#endif