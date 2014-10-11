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
#include "resource/MotionData.h"
#include "resource/Animation.h"
#include "BinReader.h"
#include "Core.h"
#include "ResourceCache.h"

enum MotionDataFlags
{
    kNoMods = 1,
    kFromDefault = 2
};

MotionData::MotionData()
{}

MotionData::MotionData(MotionData&& other)
{
    animInfos = move(other.animInfos);
}

MotionData& MotionData::operator=(MotionData&& other)
{
    animInfos = move(other.animInfos);
    return *this;
}

static void read(BinReader& reader, AnimInfo& animInfo)
{
    uint32_t animId = reader.readInt();
    animInfo.resource = Core::get().resourceCache().get(animId);
    animInfo.firstFrame = reader.readInt();
    animInfo.lastFrame = reader.readInt();
    animInfo.framesPerSecond = reader.readFloat();

    if(animInfo.lastFrame == 0xffffffff)
    {
        animInfo.lastFrame = static_cast<uint32_t>(animInfo.resource->cast<Animation>().frames.size() - 1);
    }

    if(animInfo.framesPerSecond < 0.0f && animInfo.firstFrame < animInfo.lastFrame)
    {
        swap(animInfo.firstFrame, animInfo.lastFrame);
    }    
}

void read(BinReader& reader, MotionData& animStrip)
{
    uint8_t numAnims = reader.readByte();
    animStrip.animInfos.resize(numAnims);

    uint8_t unk1 = reader.readByte();
    assert(unk1 == 0 || unk1 == 1 || unk1 == 2);

    uint8_t unk2 = reader.readByte();
    assert(unk2 == 0 || unk2 == 1 || unk2 == 2);

    uint8_t unk3 = reader.readByte();
    assert(unk3 == 0);

    for(AnimInfo& animInfo : animStrip.animInfos)
    {
        read(reader, animInfo);
    }

    if(unk2 == 1 || unk2 == 2)
    {
        reader.readFloat();
        reader.readFloat();
        reader.readFloat();
    }
}
