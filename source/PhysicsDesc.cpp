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
#include "PhysicsDesc.h"
#include "BinReader.h"
#include "Core.h"
#include "ResourceCache.h"
#include "util.h"

enum PhysicsDescFlags
{
    kCSetup = 0x00000001,
    kMTable = 0x00000002,
    kVelocity = 0x00000004,
    kAcceleration = 0x00000008,
    kOmega = 0x00000010,
    kParent = 0x00000020,
    kChildren = 0x00000040,
    kObjScale = 0x00000080,
    kFriction = 0x00000100,
    kElasticity = 0x00000200,
    kSTable = 0x00000800,
    kPETable = 0x00001000,
    kDefaultScript = 0x00002000,
    kDefaultScriptIntensity = 0x00004000,
    kPosition = 0x00008000,
    kMovement = 0x00010000,
    kAnimFrameId = 0x00020000,
    kTranslucency = 0x00040000
};

PhysicsDesc::Relation::Relation() : objectId(0), slot(0)
{}

PhysicsDesc::PhysicsDesc() : animFrameId(0)
{}

static void read(BinReader& reader, PhysicsDesc::Relation& relation)
{
    relation.objectId = reader.readInt();
    relation.slot = reader.readInt();
}

void read(BinReader& reader, PhysicsDesc& physicsDesc)
{
    uint32_t flags = reader.readInt();
    /*unknown*/ reader.readInt();

    if(flags & kMovement)
    {
        uint32_t movementSize = reader.readInt();
        reader.readRaw(movementSize);
        /*autonomousMovement*/ reader.readInt();
    }

    if(flags & kAnimFrameId)
    {
        physicsDesc.animFrameId = reader.readInt();
    }

    if(flags & kPosition)
    {
        physicsDesc.landcell = LandcellId(reader.readInt());
        read(reader, physicsDesc.location.position);
        read(reader, physicsDesc.location.rotation);
    }

    if(flags & kMTable)
    {
        physicsDesc.motionTable = Core::get().resourceCache().get(reader.readInt());
    }

    if(flags & kSTable)
    {
        physicsDesc.soundTable = Core::get().resourceCache().get(reader.readInt());
    }

    if(flags & kPETable)
    {
        physicsDesc.particleEmitterTable = Core::get().resourceCache().get(reader.readInt());
    }

    if(flags & kCSetup)
    {
        physicsDesc.setup = Core::get().resourceCache().get(reader.readInt());
    }

    if(flags & kParent)
    {
        read(reader, physicsDesc.parent);
    }

    if(flags & kChildren)
    {
        physicsDesc.children.resize(reader.readInt());

        for(PhysicsDesc::Relation& child : physicsDesc.children)
        {
            read(reader, child);
        }
    }

    if(flags & kObjScale)
    {
        physicsDesc.scale = reader.readFloat();
    }

    if(flags & kFriction)
    {
        physicsDesc.friction = reader.readFloat();
    }

    if(flags & kElasticity)
    {
        physicsDesc.elasticity = reader.readFloat();
    }

    if(flags & kTranslucency)
    {
        physicsDesc.translucency = reader.readFloat();
    }

    if(flags & kVelocity)
    {
        read(reader, physicsDesc.velocity);
    }

    if(flags & kAcceleration)
    {
        read(reader, physicsDesc.acceleration);
    }

    if(flags & kOmega)
    {
        read(reader, physicsDesc.angularVelocity);
    }

    if(flags & kDefaultScript)
    {
        physicsDesc.defaultScript = Core::get().resourceCache().get(reader.readInt());
    }

    if(flags & kDefaultScriptIntensity)
    {
        physicsDesc.defaultScriptIntensity = reader.readFloat();
    }

    /*timestamps*/ reader.readRaw(sizeof(uint16_t) * 9);

    reader.align();
}
