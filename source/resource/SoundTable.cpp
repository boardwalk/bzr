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
#include "resource/SoundTable.h"
#include "BinReader.h"

static void read(BinReader& reader, SoundData& soundData)
{
    soundData.soundId = reader.readInt();
    assert(soundData.soundId == 0 || (soundData.soundId & 0xFF000000) == ResourceType::kSound);

    soundData.priority = reader.readFloat();
    assert(soundData.priority >= 0.0 && soundData.priority <= 1.0);

    soundData.probability = reader.readFloat();
    assert(soundData.probability >= 0.0 && soundData.probability <= 1.0);

    soundData.volume = reader.readFloat();
    assert(soundData.volume >= 0.0 && soundData.volume <= 10.0);
}

static void read(BinReader& reader, SoundTableData& soundTableData)
{
    uint32_t numData = reader.readInt();
    soundTableData.data.resize(numData);

    for(auto& soundData : soundTableData.data)
    {
        read(reader, soundData);
    }
}

SoundTable::SoundTable(uint32_t id, const void* data, size_t size) : ResourceImpl(id)
{
    BinReader reader(data, size);

    uint32_t resourceId = reader.readInt();
    assert(resourceId == id);

    uint32_t unk1 = reader.readInt();
    assert(unk1 == 0);

    SoundTableData defaultData;
    read(reader, defaultData);

    uint32_t soundTableSize = reader.readInt();

    for(uint32_t i = 0; i < soundTableSize; i++)
    {
        uint32_t soundType = reader.readInt();

        read(reader, soundTable[soundType]);

        uint32_t unk = reader.readInt();
        assert(unk == 0);
    }

    assert(reader.remaining() == 0);
}
