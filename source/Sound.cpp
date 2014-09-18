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
#include "Sound.h"
#include "BinReader.h"

Sound::Sound(uint32_t id, const void* data, size_t size) : ResourceImpl(id)
{
    BinReader reader(data, size);

    uint32_t resourceId = reader.read<uint32_t>();
    assert(resourceId == id);

    uint32_t type = reader.read<uint32_t>();

    // There's one file that has type != 18
    // I'm going to ignore it till it comes up
    if(type != 18)
    {
        throw runtime_error("Type 18 sounds not supported");
    }

    uint32_t samplesLen = reader.read<uint32_t>();

    uint16_t unk1 = reader.read<uint16_t>();
    assert(unk1 == 1);

    uint16_t unk2 = reader.read<uint16_t>();
    assert(unk2 == 1 || unk2 == 2);

    samplesPerSecond_ = reader.read<uint32_t>();
    uint32_t totalSamplesPerSecond = reader.read<uint32_t>();

    numChannels_ = reader.read<uint16_t>();
    assert(numChannels_ == 1 || numChannels_ == 2 || numChannels_ == 4);
    assert(samplesPerSecond_ * numChannels_ == totalSamplesPerSecond);

    bitsPerSample_ = reader.read<uint32_t>();
    assert(bitsPerSample_ == 8 || bitsPerSample_ == 16);

    const uint8_t* samples = reader.readPointer<uint8_t>(samplesLen);

    reader.assertEnd();

    samples_.assign(samples, samples + samplesLen);
}

uint32_t Sound::samplesPerSecond() const
{
    return samplesPerSecond_;
}

uint32_t Sound::numChannels() const
{
    return numChannels_;
}

uint32_t Sound::bitsPerSample() const
{
    return bitsPerSample_;
}

const vector<uint8_t>& Sound::samples() const
{
    return samples_;
}
