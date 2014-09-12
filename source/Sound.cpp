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

    auto resourceId = reader.read<uint32_t>();
    assert(resourceId == id);

    // There's one file that has type != 18
    // I'm going to ignore it till it comes up
    auto type = reader.read<uint32_t>();
    assert(type == 18);

    auto samplesLen = reader.read<uint32_t>();

    auto unk1 = reader.read<uint16_t>();
    assert(unk1 == 1);

    auto unk2 = reader.read<uint16_t>();
    assert(unk2 == 1 || unk2 == 2);

    _samplesPerSecond = reader.read<uint32_t>();
    auto totalSamplesPerSecond = reader.read<uint32_t>();

    _numChannels = reader.read<uint32_t>();
    assert(_numChannels == 1 || _numChannels == 2 || _numChannels == 4);
    assert(_samplesPerSecond * _numChannels == totalSamplesPerSecond);

    _bitsPerSample = reader.read<uint32_t>();
    assert(_bitsPerSample == 8 || _bitsPerSample == 16);

    auto samples = reader.readPointer<uint8_t>(samplesLen);

    reader.assertEnd();

    _samples.assign(samples, samples + samplesLen);
}

uint32_t Sound::samplesPerSecond() const
{
    return _samplesPerSecond;
}

uint32_t Sound::numChannels() const
{
    return _numChannels;
}

uint32_t Sound::bitsPerSample() const
{
    return _bitsPerSample;
}

const vector<uint8_t>& Sound::samples() const
{
    return _samples;
}
