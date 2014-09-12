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

AnimationSet::AnimationSet(uint32_t id, const void* data, size_t size) : ResourceImpl(id)
{
    BinReader reader(data, size);

    auto resourceId = reader.read<uint32_t>();
    assert(resourceId == id);

    reader.read<uint32_t>();
    auto numUnknown = reader.read<uint32_t>();
    
    for(auto ui = 0u; ui < numUnknown; ui++)
    {
        reader.read<uint32_t>();
        reader.read<uint32_t>();
    }

    auto numStrips1 = reader.read<uint32_t>();
    _strips1.resize(numStrips1);

    for(auto& strip : _strips1)
    {
        strip.read(reader);
    }

    auto numStrips2 = reader.read<uint32_t>();
    _strips2.resize(numStrips2);

    for(auto& strip : _strips2)
    {
        strip.read(reader);
    }

    auto numComboStrips = reader.read<uint32_t>();
    _comboStrips.resize(numComboStrips);

    for(auto& comboStrip : _comboStrips)
    {
        reader.read<uint32_t>();

        auto numStrips3 = reader.read<uint32_t>();
        comboStrip.resize(numStrips3);

        for(auto& strip : comboStrip)
        {
            strip.read(reader);
        }
    }

    reader.assertEnd();
}