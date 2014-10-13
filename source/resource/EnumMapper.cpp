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
#include "resource/EnumMapper.h"
#include "BinReader.h"
#include "Core.h"
#include "ResourceCache.h"

EnumMapper::EnumMapper(uint32_t id, const void* data, size_t size) : ResourceImpl{id}
{
    BinReader reader(data, size);

    uint32_t resourceId = reader.readInt();
    assert(resourceId == id);

    uint32_t baseMapperId = reader.readInt();

    if(baseMapperId)
    {
        baseMapper = Core::get().resourceCache().get(baseMapperId);
        assert(baseMapper->resourceType() == ResourceType::kEnumMapper);
    }

    uint8_t unk = reader.readByte();
    assert(unk <= 7);

    uint16_t numPairs = reader.readPackedShort();
    mapping.reserve(numPairs);

    for(uint16_t i = 0; i < numPairs; i++)
    {
        uint32_t key = reader.readInt();

        uint8_t valueSize = reader.readByte();
        const uint8_t* valueBytes = reader.readRaw(valueSize);

        string value(valueBytes, valueBytes + valueSize);

        mapping[key] = move(value);
    }

    assert(reader.remaining() == 0);
}
