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
#include "resource/Sound.h"
#include "BinReader.h"

Sound::Sound(uint32_t id, const void* data, size_t size) : ResourceImpl{id}
{
    BinReader reader(data, size);

    uint32_t resourceId = reader.readInt();
    assert(resourceId == id);

    uint32_t type = reader.readInt();

    // There's one file that has type != 18
    // I'm going to ignore it till it comes up
    if(type != 18)
    {
        throw runtime_error("Type 18 sounds not supported");
    }

    uint32_t samplesLen = reader.readInt();

    uint16_t unk1 = reader.readShort();
    assert(unk1 == 1);

    uint16_t unk2 = reader.readShort();
    assert(unk2 == 1 || unk2 == 2);

    samplesPerSecond = reader.readInt();
    uint32_t totalSamplesPerSecond = reader.readInt();

    numChannels = reader.readShort();
    assert(numChannels == 1 || numChannels == 2 || numChannels == 4);
    assert(samplesPerSecond * numChannels == totalSamplesPerSecond);

    bitsPerSample = reader.readInt();
    assert(bitsPerSample == 8 || bitsPerSample == 16);

    const uint8_t* samplesPtr = reader.readRaw(samplesLen);

    assert(reader.remaining() == 0);

    samples.assign(samplesPtr, samplesPtr + samplesLen);
}
