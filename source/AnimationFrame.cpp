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
#include "AnimationFrame.h"
#include "BinReader.h"

enum AnimHooks
{
    kNoOp = 0x0000,
    kSound = 0x0001,
    kSoundTable = 0x0002,
    kAttack = 0x0003,
    kAnimDone = 0x0004,
    kReplaceObject = 0x0005,
    kEthereal = 0x0006,
    kTransparentPart = 0x0007,
    kLuminous = 0x0008,
    kLuminousPart = 0x0009,
    kDiffuse = 0x000a,
    kDiffusePart = 0x000b,
    kScale = 0x000c,
    kCreateParticle = 0x000d,
    kDestroyParticle = 0x000e,
    kStopParticle = 0x000f,
    kNoDraw = 0x0010,
    kDefaultScript = 0x0011,
    kDefaultScriptPart = 0x0012,
    kCallPes = 0x0013,
    kTransparent = 0x0014,
    kSoundTweaked = 0x0015,
    kSetOmega = 0x0016,
    kTextureVelocity = 0x0017,
    kTextureVelocityPart = 0x0018,
    kSetLight = 0x0019,
    kCreateBlockingParticle = 0x001a
};

AnimationFrame::AnimationFrame(BinReader& reader, uint32_t numModels)
{
    orientations.resize(numModels);

    for(Orientation& orientation : orientations)
    {
        orientation.position.x = reader.readFloat();
        orientation.position.y = reader.readFloat();
        orientation.position.z = reader.readFloat();

        orientation.rotation.w = reader.readFloat();
        orientation.rotation.x = reader.readFloat();
        orientation.rotation.y = reader.readFloat();
        orientation.rotation.z = reader.readFloat();
    }

    uint32_t numHooks = reader.readInt();

    for(uint32_t hi = 0; hi < numHooks; hi++)
    {
        uint32_t hookType = reader.readInt();
        uint32_t hookSize = 0;

        switch(hookType)
        {
            case kSound: hookSize = 2;  break; // 0x00,soundref
            case kSoundTable: hookSize = 2;  break; // 0x00,0x0C
            case kAttack: hookSize = 8;  break; // 0x00,0x14,6floats
            case kReplaceObject: hookSize = 2;  break; // 0x00,0xBB401
            case kEthereal: hookSize = 2;  break; // 0x01,0x01
            case kTransparentPart: hookSize = 5;  break; // 0x00,0x0A,1.0,1.0,0x00
            case kCreateParticle: hookSize = 11; break; // lotsa stuff (3 floats in there somewhere)
            case kStopParticle: hookSize = 2;  break; // 0x00,0x01
            case kDefaultScript: hookSize = 1;  break; // 0x00
            case kCallPes: hookSize = 3;  break; // 0x00,someREF,0x00
            case kTransparent: hookSize = 4;  break; // 0x00,0x00,0x00,0x00
            case kSoundTweaked: hookSize = 5;  break; // 0x00,soundref,3floats
            case kSetOmega: hookSize = 4;  break; // 0x00,0x00,2floats
            default:
                throw runtime_error("Unknown hookType in animation frame");
        }

        reader.readRaw(hookSize * sizeof(uint32_t));
    }
}

AnimationFrame::AnimationFrame(AnimationFrame&& other)
{
    orientations = move(other.orientations);
}

AnimationFrame& AnimationFrame::operator=(AnimationFrame&& other)
{
    orientations = move(other.orientations);
    return *this;
}
