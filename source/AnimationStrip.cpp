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
#include "Animation.h"
#include "BinReader.h"
#include "Core.h"
#include "ResourceCache.h"

AnimationStrip::AnimationStrip(BinReader& reader)
{
    id = reader.read<uint16_t>();
    stanceId = reader.read<uint16_t>();

    uint8_t numAnims = reader.read<uint8_t>();
    animInfos.resize(numAnims);

    uint8_t unk1 = reader.read<uint8_t>();
    assert(unk1 == 0 || unk1 == 1 || unk1 == 2);

    uint8_t unk2 = reader.read<uint8_t>();
    assert(unk2 == 0 || unk2 == 1 || unk2 == 2);

    uint8_t unk3 = reader.read<uint8_t>();
    assert(unk3 == 0);

    for(AnimInfo& animInfo : animInfos)
    {
        uint32_t animId = reader.read<uint32_t>();
        animInfo.resource = Core::get().resourceCache().get(animId);
        animInfo.firstFrame = reader.read<uint32_t>();
        animInfo.lastFrame = reader.read<uint32_t>();
        animInfo.framesPerSecond = reader.read<float>();

        if(animInfo.lastFrame == 0xffffffff)
        {
            animInfo.lastFrame = static_cast<uint32_t>(animInfo.resource->cast<Animation>().frames.size() - 1);
        }

        if(animInfo.framesPerSecond < 0.0f && animInfo.firstFrame < animInfo.lastFrame)
        {
            swap(animInfo.firstFrame, animInfo.lastFrame);
        }
    }

    if(unk2 == 1 || unk2 == 2)
    {
        reader.read<float>();
        reader.read<float>();
        reader.read<float>();
    }
}

AnimationStrip::AnimationStrip(AnimationStrip&& other)
{
    id = other.id;
    stanceId = other.stanceId;
    animInfos = move(other.animInfos);
}

AnimationStrip& AnimationStrip::operator=(AnimationStrip&& other)
{
    id = other.id;
    stanceId = other.stanceId;
    animInfos = move(other.animInfos);
    return *this;
}
