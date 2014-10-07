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
    kCallPES = 0x0013,
    kTransparent = 0x0014,
    kSoundTweaked = 0x0015,
    kSetOmega = 0x0016,
    kTextureVelocity = 0x0017,
    kTextureVelocityPart = 0x0018,
    kSetLight = 0x0019,
    kCreateBlockingParticle = 0x001a
};

AnimationFrame::AnimationFrame()
{}

AnimationFrame::AnimationFrame(AnimationFrame&& other)
{
    locations = move(other.locations);
}

AnimationFrame& AnimationFrame::operator=(AnimationFrame&& other)
{
    locations = move(other.locations);
    return *this;
}

void read(BinReader& reader, AnimationFrame& frame, uint32_t numModels)
{
    frame.locations.resize(numModels);

    for(Location& loc : frame.locations)
    {
        read(reader, loc);
    }

    uint32_t numHooks = reader.readInt();

    for(uint32_t i = 0; i < numHooks; i++)
    {
        uint32_t hookType = reader.readInt();
        uint32_t hookSize = 0;

        // only apply this hook when playing this frame
        // 1 = forward, -1 = backward, 0 = both
        uint32_t hookDir = reader.readInt();
        assert(hookDir == 0 || hookDir == 1 || hookDir == 0xFFFFFFFF);

        switch(hookType)
        {
            case kSound:
                /* struct SoundHook
                   uint32_t sound_gid; */
                hookSize = 1;
                break;

            case kSoundTable:
                /* struct SoundTableHook
                   uint32_t sound_type; */
                hookSize = 1;
                break;

            case kAttack:
                /* struct AttackHook
                   struct AttackCone
                   uint32_t part_index;
                   float left_x;
                   float left_y;
                   float right_x;
                   float right_y;
                   float radius;
                   float height; */
                hookSize = 7;
                break;

            case kReplaceObject:
                /* struct ReplaceObjectHook
                   struct AnimPartChange
                   uint16_t part_index;
                   uint16_t part_gid; */
                hookSize = 1;
                break;

            case kEthereal:
                /* struct EtherealHook
                   uint32_t ethereal; */
                hookSize = 1;
                break;

            case kTransparentPart:
                /* struct TransparentPartHook
                   uint32_t part_index;
                   float start;
                   float end;
                   float time; */
                hookSize = 4;
                break;

            case kCreateParticle:
                /* struct CreateParticleHook
                   uint32_t emitter_info_gid;
                   uint32_t part_index;
                   struct Frame offset
                   float px, py, pz;
                   float rw, rx, ry, rz;
                   uint32_t emitter_id; */
                hookSize = 10;
                break;

            case kStopParticle:
                /* struct StopParticleHook
                   uint32_t emitter_id; */
                hookSize = 1;
                break;

            case kDefaultScript:
                /* struct DefaultScriptHook */
                hookSize = 0;
                break;

            case kCallPES:
                /* struct CallPESHook
                   uint32_t pes_gid;
                   float pause; */
                hookSize = 2;
                break;

            case kTransparent:
                /* struct TransparentHook
                   float start;
                   float end;
                   float time; */
                hookSize = 3;
                break;

            case kSoundTweaked:
                /* struct SoundTweakedHook
                   uint32_t sound_gid;
                   float priority;
                   float probability;
                   float volume; */
                hookSize = 4;
                break;

            case kSetOmega:
                /* struct SetOmegaHook
                   struct Vector3 axis
                   float x, y, z; */
                hookSize = 3;
                break;

            default:
                throw runtime_error("Unknown hookType in animation frame");
        }

        reader.readRaw(hookSize * sizeof(uint32_t));
    }
}
