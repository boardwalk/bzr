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
#include "resource/ParticleEmitter.h"

enum class EmitterType : uint32_t
{
    kUnknown = 0x0000,
    kBirthratePerSec = 0x0001,
    kBirthratePerMeter = 0x0002
};

enum class ParticleType : uint32_t
{
    kUnknown = 0x0000,
    kStill = 0x0001,
    kLocalVelocity = 0x0002,
    kParabolicLVGA = 0x0003,
    kParabolicLVGAGR = 0x0004,
    kSwarm = 0x0005,
    kExplode = 0x0006,
    kImplode = 0x0007,
    kParabolicLVLA = 0x0008,
    kParabolicLVLALR = 0x0009,
    kParabolicGVGA = 0x000a,
    kParabolicGVGAGR = 0x000b,
    kGlobalVelocity = 0x000c
};

ParticleEmitter::ParticleEmitter(uint32_t id, const void* data, size_t size) : ResourceImpl{id}
{
    UNUSED(data);
    UNUSED(size);
}