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
#include "TriangleFan.h"
#include "BinReader.h"

void TriangleFan::read(BinReader& reader)
{
    uint8_t numIndices = reader.readByte();
    indices.resize(numIndices);

    flags = reader.readByte();
    assert(flags == 0x0 || flags == 0x1 || flags == 0x4);

    uint32_t flags2 = reader.readInt();
    assert(flags2 == 0x0 || flags2 == 0x1 || flags2 == 0x2);

    texIndex = reader.readShort();

    reader.readShort();

    for(Index& index : indices)
    {
        index.vertexIndex = reader.readShort();
    }

    if(flags != 0x04)
    {
        for(Index& index : indices)
        {
            index.texCoordIndex = reader.readByte();
        }
    }

    if(flags2 == 0x02)
    {
        for(uint8_t pvi = 0; pvi < numIndices; pvi++)
        {
            reader.readByte();
        }
    }
}
