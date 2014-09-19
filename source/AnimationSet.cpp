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
#include "AnimationSet.h"
#include "BinReader.h"

AnimationSet::AnimationSet(uint32_t id, const void* data, size_t size) : ResourceImpl{id}
{
    BinReader reader(data, size);

    uint32_t resourceId = reader.read<uint32_t>();
    assert(resourceId == id);

    reader.read<uint32_t>();
    uint32_t numUnknown = reader.read<uint32_t>();

    for(uint32_t ui = 0; ui < numUnknown; ui++)
    {
        reader.read<uint32_t>();
        reader.read<uint32_t>();
    }

    uint32_t numStrips1 = reader.read<uint32_t>();
    strips1.reserve(numStrips1);

    for(uint32_t si = 0; si < numStrips1; si++)
    {
        strips1.emplace_back(reader);
    }

    uint32_t numStrips2 = reader.read<uint32_t>();
    strips2.reserve(numStrips2);

    for(uint32_t si = 0; si < numStrips2; si++)
    {
        strips2.emplace_back(reader);
    }

    uint32_t numComboStrips = reader.read<uint32_t>();
    comboStrips.resize(numComboStrips);

    for(vector<AnimationStrip>& comboStrip : comboStrips)
    {
        reader.read<uint32_t>();

        uint32_t numStrips3 = reader.read<uint32_t>();
        comboStrip.reserve(numStrips3);

        for(uint32_t si = 0; si < numStrips3; si++)
        {
            comboStrip.emplace_back(reader);
        }
    }

    reader.assertEnd();
}
