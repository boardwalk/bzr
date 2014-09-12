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
#include "AnimationStrip.h"
#include "BinReader.h"
#include "Core.h"
#include "ResourceCache.h"

AnimationStrip::AnimationStrip() : _id(0), _stanceId(0)
{}

AnimationStrip::AnimationStrip(AnimationStrip&& other)
{
    _id = other._id;
    _stanceId = other._stanceId;
    _animInfos = move(other._animInfos);
}

void AnimationStrip::read(BinReader& reader)
{
    _id = reader.read<uint16_t>();
    _stanceId = reader.read<uint16_t>();

    auto numAnims = reader.read<uint8_t>();
    _animInfos.resize(numAnims);

    auto unk1 = reader.read<uint8_t>();
    assert(unk1 == 0 || unk1 == 1 || unk1 == 2);

    auto unk2 = reader.read<uint8_t>();
    assert(unk2 == 0 || unk2 == 1 || unk2 == 2);

    auto unk3 = reader.read<uint8_t>();
    assert(unk3 == 0);

    for(auto& animInfo : _animInfos)
    {
        auto animId = reader.read<uint32_t>();
        animInfo.resource = Core::get().resourceCache().get(animId);

        reader.read<uint32_t>();
        reader.read<uint32_t>();

        animInfo.playSpeed = reader.read<float>();
    }

    if(unk2 == 1 || unk2 == 2)
    {
        reader.read<uint32_t>();
        reader.read<uint32_t>();
        reader.read<uint32_t>();
    }
}

uint16_t AnimationStrip::id() const
{
    return _id;
}

uint16_t AnimationStrip::stanceId() const
{
    return _stanceId;
}

const vector<AnimationStrip::AnimInfo>& AnimationStrip::animInfos() const
{
    return _animInfos;
}