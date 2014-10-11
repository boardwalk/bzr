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
#ifndef BZR_ANIMATIONHOOK_H
#define BZR_ANIMATIONHOOK_H

class BinReader;

struct AnimationHook
{
    virtual ~AnimationHook() {}
};

// AC: SoundHook
struct SoundHook : public AnimationHook
{
    uint32_t soundId;
};

// AC: SoundTableHook
struct SoundTableHook : public AnimationHook
{
    uint32_t soundType;
};

// AC: AttackHook
// AC: AttackCone
struct AttackHook : public AnimationHook
{
    uint32_t partIndex;
    float leftX;
    float leftY;
    float rightX;
    float rightY;
    float radius;
    float height;
};

// AC: ReplaceObjectHook
// AC: AnimPartChange
struct ReplaceObjectHook : public AnimationHook
{
    uint16_t partIndex;
    uint16_t partId;
};

// AC: EtherealHook
struct EtherealHook : public AnimationHook
{
    bool ethereal;
};

// AC: TransparentPartHook
struct TransparentPartHook : public AnimationHook
{
    uint32_t partIndex;
    float start;
    float end;
    float time;
};

// AC: ScaleHook
struct ScaleHook : public AnimationHook
{
    float end;
    float time;
};

// AC: CreateParticleHook
struct CreateParticleHook : public AnimationHook
{
    uint32_t emitterInfoId;
    uint32_t partIndex;
    glm::vec3 position;
    glm::quat rotation;
    uint32_t emitterId;
};

// AC: DestroyParticleHook
struct DestroyParticleHook : public AnimationHook
{
    uint32_t emitterId;
};

// AC: StopParticleHook
struct StopParticleHook : public AnimationHook
{
    uint32_t emitterId;
};

// AC: NoDrawHook
struct NoDrawHook : public AnimationHook
{
    bool noDraw;
};

// AC: DefaultScriptHook
struct DefaultScriptHook : public AnimationHook
{
};

// AC: CellPESHook
struct CallPESHook : public AnimationHook
{
    uint32_t pesId;
    float pause;
};

// AC: TransparentHook
struct TransparentHook : public AnimationHook
{
    float start;
    float end;
    float time;
};

// AC: SoundTweakedHook
struct SoundTweakedHook : public AnimationHook
{
    uint32_t soundId;
    float priority;
    float probability;
    float volume;
};

// AC: SetOmegaHook
struct SetOmegaHook : public AnimationHook
{
    glm::vec3 axis;
};

// AC: TextureVelocityHook
struct TextureVelocityHook : public AnimationHook
{
    float uSpeed;
    float vSpeed;
};

// AC: SetLightHook
struct SetLightHook : public AnimationHook
{
    bool lightsOn;
};

// AC: CreateBlockingParticleHook
struct CreateBlockingParticleHook : public AnimationHook
{
    uint32_t emitterInfoId;
    uint32_t partIndex;
    glm::vec3 position;
    glm::quat rotation;
    uint32_t emitterId;
};

void read(BinReader& reader, unique_ptr<AnimationHook>& hook);

#endif
