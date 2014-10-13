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
#include "resource/AnimationHook.h"
#include "BinReader.h"
#include "util.h"

enum class AnimationHookType : uint32_t
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

void read(BinReader& reader, unique_ptr<AnimationHook>& hook)
{
    uint32_t hookType = reader.readInt();

    // only apply this hook when playing this frame
    // 1 = forward, -1 = backward, 0 = both
    uint32_t hookDir = reader.readInt();
    assert(hookDir == 0 || hookDir == 1 || hookDir == 0xFFFFFFFF);
    UNUSED(hookDir);

    switch(hookType)
    {
        case AnimationHookType::kSound:
            {
                SoundHook* h = new SoundHook();
                hook.reset(h);
                h->soundId = reader.readInt();
            }
            break;

        case AnimationHookType::kSoundTable:
            {
                SoundTableHook* h = new SoundTableHook();
                hook.reset(h);
                h->soundType = reader.readInt();
            }
            break;

        case AnimationHookType::kAttack:
            {
                AttackHook* h = new AttackHook();
                hook.reset(h);
                h->partIndex = reader.readInt();
                h->leftX = reader.readFloat();
                h->leftY = reader.readFloat();
                h->rightX = reader.readFloat();
                h->rightY = reader.readFloat();
                h->radius = reader.readFloat();
                h->height = reader.readFloat();
            }
            break;

        case AnimationHookType::kReplaceObject:
            {
                ReplaceObjectHook* h = new ReplaceObjectHook();
                hook.reset(h);
                h->partIndex = reader.readShort();
                h->partId = reader.readShort();
            }
            break;

        case AnimationHookType::kEthereal:
            {
                EtherealHook* h = new EtherealHook();
                hook.reset(h);
                h->ethereal = (reader.readInt() != 0);
            }
            break;

        case AnimationHookType::kTransparentPart:
            {
                TransparentPartHook* h = new TransparentPartHook();
                hook.reset(h);
                h->partIndex = reader.readInt();
                h->start = reader.readFloat();
                h->end = reader.readFloat();
                h->time = reader.readFloat();
            }
            break;

        case AnimationHookType::kScale:
            {
                ScaleHook* h = new ScaleHook();
                hook.reset(h);
                h->end = reader.readFloat();
                h->time = reader.readFloat();
            }
            break;

        case AnimationHookType::kCreateParticle:
            {
                CreateParticleHook* h = new CreateParticleHook();
                hook.reset(h);
                h->emitterInfoId = reader.readInt();
                h->partIndex = reader.readInt();
                read(reader, h->position);
                read(reader, h->rotation);
                h->emitterId = reader.readInt();
            }
            break;

        case AnimationHookType::kDestroyParticle:
            {
                DestroyParticleHook* h = new DestroyParticleHook();
                hook.reset(h);
                h->emitterId = reader.readInt();
            }
            break;

        case AnimationHookType::kStopParticle:
            {
                StopParticleHook* h = new StopParticleHook();
                hook.reset(h);
                h->emitterId = reader.readInt();
            }
            break;

        case AnimationHookType::kNoDraw:
            {
                NoDrawHook* h = new NoDrawHook();
                hook.reset(h);
                h->noDraw = (reader.readInt() != 0);
            }
            break;

        case AnimationHookType::kDefaultScript:
            {
                DefaultScriptHook* h = new DefaultScriptHook();
                hook.reset(h);
            }
            break;

        case AnimationHookType::kCallPES:
            {
                CallPESHook* h = new CallPESHook();
                hook.reset(h);
                h->pesId = reader.readInt();
                h->pause = reader.readFloat();
            }
            break;

        case AnimationHookType::kTransparent:
            {
                TransparentHook* h = new TransparentHook();
                hook.reset(h);
                h->start = reader.readFloat();
                h->end = reader.readFloat();
                h->time = reader.readFloat();
            }
            break;

        case AnimationHookType::kSoundTweaked:
            {
                SoundTweakedHook* h = new SoundTweakedHook();
                hook.reset(h);
                h->soundId = reader.readInt();
                h->priority = reader.readFloat();
                h->probability = reader.readFloat();
                h->volume = reader.readFloat();
            }
            break;

        case AnimationHookType::kSetOmega:
            {
                SetOmegaHook* h = new SetOmegaHook();
                hook.reset(h);
                read(reader, h->axis);
            }
            break;

        case AnimationHookType::kTextureVelocity:
            {
                TextureVelocityHook* h = new TextureVelocityHook();
                hook.reset(h);
                h->uSpeed = reader.readFloat();
                h->vSpeed = reader.readFloat();
            }
            break;

        case AnimationHookType::kSetLight:
            {
                SetLightHook* h = new SetLightHook();
                hook.reset(h);
                h->lightsOn = (reader.readInt() != 0);
            }
            break;

        case AnimationHookType::kCreateBlockingParticle:
            {
                CreateBlockingParticleHook* h = new CreateBlockingParticleHook();
                hook.reset(h);
                h->emitterInfoId = reader.readInt();
                h->partIndex = reader.readInt();
                read(reader, h->position);
                read(reader, h->rotation);
                h->emitterId = reader.readInt();
            }
            break;

        default:
            throw runtime_error("Unknown hookType in animation frame");
    }
}
