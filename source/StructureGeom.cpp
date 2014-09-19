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
#include "StructureGeom.h"
#include "BinReader.h"

StructureGeom::StructureGeom(uint32_t id, const void* data, size_t size) : ResourceImpl{id}
{
    BinReader reader(data, size);

    uint32_t resourceId = reader.read<uint32_t>();
    assert(resourceId == id);

    uint32_t numParts = reader.read<uint32_t>();
    parts.resize(numParts);

    for(uint32_t pi = 0; pi < numParts; pi++)
    {
        uint32_t partNum = reader.read<uint32_t>();
        assert(partNum == pi);

        parts[pi].read(reader);
    }

    reader.assertEnd();
}
