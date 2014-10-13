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
#include "BinReader.h"

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
    BinReader reader(data, size);

    uint32_t resourceId = reader.readInt();
    assert(resourceId == id);
    UNUSED(resourceId);

    uint32_t unk1 = reader.readInt();
    assert(unk1 == 0);
    UNUSED(unk1);

    uint32_t emitterType = reader.readInt();
    assert(emitterType >= 0x1 && emitterType <= 0x2);
    UNUSED(emitterType);

    uint32_t particleType = reader.readInt();
    assert(particleType >= 0x1 && particleType <= 0xc);
    UNUSED(particleType);

    uint32_t gfxobjId = reader.readInt();
    assert(gfxobjId == 0 || (gfxobjId & 0xFF000000) == static_cast<uint32_t>(ResourceType::kModel));
    UNUSED(gfxobjId);

    uint32_t hwGfxobjId = reader.readInt();
    assert(hwGfxobjId == 0 || (hwGfxobjId & 0xFF000000) == static_cast<uint32_t>(ResourceType::kModel));
    UNUSED(hwGfxobjId);

    /*birthrate*/ reader.readDouble();

    /*maxParticles*/ reader.readInt();
    /*initialParticles*/ reader.readInt();
    /*totalParticles*/ reader.readInt();

    /*totalSeconds*/ reader.readDouble();
    /*lifespanRand*/ reader.readDouble();
    /*lifespan*/ reader.readDouble();

    // sorting sphere
    /*r*/ reader.readFloat();

    // offsetDir
    /*x*/ reader.readFloat();
    /*y*/ reader.readFloat();
    /*z*/ reader.readFloat();

    /*minOffset*/ reader.readFloat();
    /*maxOffset*/ reader.readFloat();

    // a
    /*x*/ reader.readFloat();
    /*y*/ reader.readFloat();
    /*z*/ reader.readFloat();

    // b
    /*x*/ reader.readFloat();
    /*y*/ reader.readFloat();
    /*z*/ reader.readFloat();

    // c
    /*x*/ reader.readFloat();
    /*y*/ reader.readFloat();
    /*z*/ reader.readFloat();

    /*minA*/ reader.readFloat();
    /*maxA*/ reader.readFloat();

    /*minB*/ reader.readFloat();
    /*maxB*/ reader.readFloat();

    /*minC*/ reader.readFloat();
    /*maxC*/ reader.readFloat();

    /*scaleRand*/ reader.readFloat();
    /*startScale*/ reader.readFloat();
    /*finalScale*/ reader.readFloat();

    /*transRand*/ reader.readFloat();
    /*startTrans*/ reader.readFloat();
    /*finalTrans*/ reader.readFloat();

    assert(reader.remaining() == 0);
}
