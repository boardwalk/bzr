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
#include "BinReader.h"
#include "Core.h"
#include "Log.h"

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

enum PublicWeenieDescFlags
{
    kPluralName = 0x00000001,
    kItemsCapacity = 0x00000002,
    kContainersCapacity = 0x00000004,
    kValue = 0x00000008,
    kUseability = 0x00000010,
    kUseRadius = 0x00000020,
    kMonarch = 0x00000040,
    kUIEffects = 0x00000080,
    kAmmoType = 0x00000100,
    kCombatUse = 0x00000200,
    kStructure = 0x00000400,
    kMaxStructure = 0x00000800,
    kStackSize = 0x00001000,
    kMaxStackSize = 0x00002000,
    kContainerID = 0x00004000,
    kWielderID = 0x00008000,
    kValidLocations = 0x00010000,
    kLocation = 0x00020000,
    kPriority = 0x00040000,
    kTargetType = 0x00080000,
    kBlipColor = 0x00100000,
    kBurden = 0x00200000,
    kSpellID = 0x00400000,
    kRadarEnum = 0x00800000,
    kRadarDistance = 0x01000000,
    kHouseOwner = 0x02000000,
    kHouseRestrictions = 0x04000000,
    kPScript = 0x08000000,
    kHookType = 0x10000000,
    kHookItemTypes = 0x20000000,
    kIconOverlay = 0x40000000,
    kMaterialType = 0x80000000
};

enum PublicWeenieDescFlags2
{
    kIconUnderlay = 0x0001,
    kCooldownID = 0x0002,
    kCooldownDuration = 0x0004,
    kPetOwner = 0x0008
};

static void handleVisualDesc(BinReader& reader)
{
    uint8_t eleven = reader.readByte();
    assert(eleven == 0x11);

    uint8_t paletteCount = reader.readByte();
    uint8_t textureCount = reader.readByte();
    uint8_t modelCount = reader.readByte();

    if(paletteCount != 0)
    {
        /*palette*/ reader.readPackedInt();
    }

    for(uint8_t i = 0; i < paletteCount; i++)
    {
        /*palette*/ reader.readPackedInt();
        /*offset*/ reader.readByte();
        /*length*/ reader.readByte();
    }

    for(uint8_t i = 0; i < textureCount; i++)
    {
        /*index*/ reader.readByte();
        /*old*/ reader.readPackedInt();
        /*new*/ reader.readPackedInt();
    }

    for(uint8_t i = 0; i < modelCount; i++)
    {
        /*index*/ reader.readByte();
        /*model*/ reader.readPackedInt();
    }

    reader.align();
}

static void handlePhysicsDesc(BinReader& reader)
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
        /*animFrameId*/ reader.readInt();
    }

    if(flags & kPosition)
    {
        /*position*/ reader.readRaw(32);
    }

    if(flags & kMTable)
    {
        /*motionTable*/ reader.readInt();
    }

    if(flags & kSTable)
    {
        /*soundTable*/ reader.readInt();
    }

    if(flags & kPETable)
    {
        /*particleEmitterTable*/ reader.readInt();
    }

    if(flags & kCSetup)
    {
        /*setup*/ reader.readInt();
    }

    if(flags & kParent)
    {
        /*parent*/ reader.readInt();
        /*parentSlot*/ reader.readInt();
    }

    if(flags & kChildren)
    {
        uint32_t numChildren = reader.readInt();

        for(uint32_t i = 0; i < numChildren; i++)
        {
            /*child*/ reader.readInt();
            /*childSlot*/ reader.readInt();
        }
    }

    if(flags & kObjScale)
    {
        /*scale*/ reader.readFloat();
    }

    if(flags & kFriction)
    {
        /*friction*/ reader.readFloat();
    }

    if(flags & kElasticity)
    {
        /*elasticity*/ reader.readFloat();
    }

    if(flags & kTranslucency)
    {
        /*translucency*/ reader.readFloat();
    }

    if(flags & kVelocity)
    {
        /*vx*/ reader.readFloat();
        /*vy*/ reader.readFloat();
        /*vz*/ reader.readFloat();
    }

    if(flags & kAcceleration)
    {
        /*ax*/ reader.readFloat();
        /*ay*/ reader.readFloat();
        /*az*/ reader.readFloat();
    }

    if(flags & kOmega)
    {
        /*avx*/ reader.readFloat();
        /*avy*/ reader.readFloat();
        /*avz*/ reader.readFloat();
    }

    if(flags & kDefaultScript)
    {
        /*defaultScript*/ reader.readInt();
    }

    if(flags & kDefaultScriptIntensity)
    {
        /*defaultScriptIntensity*/ reader.readFloat();
    }

    /*timestamps*/ reader.readRaw(sizeof(uint16_t) * 9);

    reader.align();
}

static void handleWeenieDesc(BinReader& reader)
{
    /*uint32_t flags = */reader.readInt();

    string name = reader.readString();
    LOG(Misc, Debug) << "  name=" << name << "\n";
}

void handleCreateObject(BinReader& reader)
{
    //reader.dump();

    /*uint32_t objectId = */reader.readInt();

    handleVisualDesc(reader);
    handlePhysicsDesc(reader);
    handleWeenieDesc(reader);
}
