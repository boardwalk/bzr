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
#include "ObjectManager.h"
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

enum BitfieldIndex
{
    kOpenable = 0x0001,
    kInscribable = 0x0002,
    kStuck = 0x0004,
    kPlayer = 0x0008,
    kAttackable = 0x0010,
    kPlayerKiller = 0x0020,
    kHiddenAdmin = 0x0040,
    kUIHidden = 0x0080,
    kBook = 0x0100,
    kVendor = 0x0200,
    kPkswitch = 0x0400,
    kNpkswitch = 0x0800,
    kDoor = 0x1000,
    kCorpse = 0x2000,
    kLifestone = 0x4000,
    kFood = 0x8000,
    kHealer = 0x00010000,
    kLockpick = 0x00020000,
    kPortal = 0x00040000,
    kAdmin = 0x00100000,
    kFreePkstatus = 0x00200000,
    kImmuneCellRestrictions = 0x00400000,
    kRequiresPackslot = 0x00800000,
    kRetained = 0x01000000,
    kPklitePkstatus = 0x02000000,
    kIncludesSecondHeader = 0x04000000,
    kBindstone = 0x08000000,
    kVolatileRare = 0x10000000,
    kWieldOnUse = 0x20000000,
    kWieldLeft = 0x40000000
};

enum PublicWeenieDescFlags2
{
    kIconUnderlay = 0x0001,
    kCooldownID = 0x0002,
    kCooldownDuration = 0x0004,
    kPetOwner = 0x0008
};

// AC: ObjDesc
static void handleObjDesc(BinReader& reader, Object&)
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

// AC: PhysicsDesc
static void handlePhysicsDesc(BinReader& reader, Object& object)
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
        object.setLandcellId(LandcellId(reader.readInt()));

        Location location;
        read(reader, location.position);
        read(reader, location.rotation);
        object.setLocation(location);
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
        uint32_t setup = reader.readInt();
        object.setModel(Core::get().resourceCache().get(setup));
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

// AC: PublicWeenieDesc
static void handleWeenieDesc(BinReader& reader, Object& object)
{
    uint32_t flags = reader.readInt();
    uint32_t flags2 = 0;

    object.setProperty(StringProperty::kName, reader.readString());
    /*weenieClassId*/ reader.readPackedInt();
    object.setProperty(DIDProperty::kIcon, reader.readPackedInt());
    object.setProperty(IntProperty::kItemType, reader.readInt()); // ITEM_TYPE enum, prefixed with TYPE_

    uint32_t bitfield = reader.readInt(); // PublicWeenieDesc::BitfieldIndex enum, prefixed with BF_

    reader.align();

    if(bitfield & kInscribable)
    {
        object.setProperty(BoolProperty::kInscribable, true);
    }

    if(bitfield & kStuck)
    {
        object.setProperty(BoolProperty::kStuck, true);
    }

    if(bitfield & kAttackable)
    {
        object.setProperty(BoolProperty::kAttackable, true);
    }

    if(bitfield & kHiddenAdmin)
    {
        object.setProperty(BoolProperty::kHiddenAdmin, true);
    }

    if(bitfield & kUIHidden)
    {
        object.setProperty(BoolProperty::kUIHidden, true);
    }

    if(bitfield & kAdmin)
    {
        object.setProperty(BoolProperty::kIsAdmin, true);
    }

    if(bitfield & kRequiresPackslot)
    {
        object.setProperty(BoolProperty::kRequiresBackpackSlot, true);
    }

    if(bitfield & kRetained)
    {
        object.setProperty(BoolProperty::kRetained, true);
    }

    if(bitfield & kWieldOnUse)
    {
        object.setProperty(BoolProperty::kWieldOnUse, true);
    }

    if(bitfield & kIncludesSecondHeader)
    {
        flags2 = reader.readInt();
    }

    if(flags & kPluralName)
    {
        object.setProperty(StringProperty::kPluralName, reader.readString());
    }

    if(flags & kItemsCapacity)
    {
        object.setProperty(IntProperty::kItemsCapacity, reader.readByte());
    }

    if(flags & kContainersCapacity)
    {
        object.setProperty(IntProperty::kContainersCapacity, reader.readByte());
    }

    if(flags & kAmmoType)
    {
        object.setProperty(IntProperty::kAmmoType, reader.readShort());
    }

    if(flags & kValue)
    {
        object.setProperty(IntProperty::kValue, reader.readInt());
    }

    if(flags & kUseability)
    {
        object.setProperty(IntProperty::kItemUseable, reader.readInt());
    }

    if(flags & kUseRadius)
    {
        object.setProperty(FloatProperty::kUseRadius, reader.readFloat());
    }

    if(flags & kTargetType)
    {
        object.setProperty(IntProperty::kTargetType, reader.readInt());
    }

    if(flags & kUIEffects)
    {
        object.setProperty(IntProperty::kUIEffects, reader.readInt());
    }

    if(flags & kCombatUse)
    {
        object.setProperty(IntProperty::kCombatUse, reader.readByte());
    }

    if(flags & kStructure)
    {
        object.setProperty(IntProperty::kStructure, reader.readShort());
    }

    if(flags & kMaxStructure)
    {
        object.setProperty(IntProperty::kMaxStructure, reader.readShort());
    }

    if(flags & kStackSize)
    {
        object.setProperty(IntProperty::kStackSize, reader.readShort());
    }

    if(flags & kMaxStackSize)
    {
        object.setProperty(IntProperty::kMaxStackSize, reader.readShort());
    }

    if(flags & kContainerID)
    {
        object.setProperty(IIDProperty::kContainer, reader.readInt());
    }

    if(flags & kWielderID)
    {
        object.setProperty(IIDProperty::kWielder, reader.readInt());
    }

    if(flags & kValidLocations)
    {
        object.setProperty(IntProperty::kLocations, reader.readInt());
    }

    if(flags & kLocation)
    {
        object.setProperty(IntProperty::kCurrentWieldedLocation, reader.readInt());
    }

    if(flags & kPriority)
    {
        object.setProperty(IntProperty::kClothingPriority, reader.readInt());
    }

    if(flags & kBlipColor)
    {
        object.setProperty(IntProperty::kRadarblipColor, reader.readByte());
    }

    if(flags & kRadarEnum)
    {
        object.setProperty(IntProperty::kShowableOnRadar, reader.readByte());
    }

    if(flags & kPScript)
    {
        object.setProperty(DIDProperty::kPhysicsScript, reader.readPackedInt());
    }

    if(flags & kRadarDistance)
    {
        object.setProperty(FloatProperty::kObviousRadarRange, reader.readFloat());
    }

    if(flags & kBurden)
    {
        object.setProperty(IntProperty::kEncumbVal, reader.readShort());
    }

    if(flags & kSpellID)
    {
        object.setProperty(DIDProperty::kSpell, reader.readShort()); // A short?
    }

    if(flags & kHouseOwner)
    {
        object.setProperty(IIDProperty::kHouseOwner, reader.readInt());
    }

    if(flags & kHouseRestrictions)
    {
        /*flags*/ reader.readInt();
        /*open*/ reader.readInt();
        /*allegiance*/ reader.readInt();
        uint16_t guestCount = reader.readShort();
        /*guestLimit*/ reader.readShort();

        for(uint16_t i = 0; i < guestCount; i++)
        {
            /*guest*/ reader.readInt();
            /*storage*/ reader.readInt();
        }
    }

    if(flags & kHookItemTypes)
    {
        object.setProperty(IntProperty::kHookPlacement, reader.readShort());
        object.setProperty(IntProperty::kHookItemType, reader.readShort());
    }

    if(flags & kMonarch)
    {
        object.setProperty(IIDProperty::kMonarch, reader.readInt());
    }

    if(flags & kHookType)
    {
        object.setProperty(IntProperty::kHookType, reader.readShort());
    }

    if(flags & kIconOverlay)
    {
        object.setProperty(DIDProperty::kIconOverlay, reader.readPackedInt());
    }

    if(flags2 & kIconUnderlay)
    {
        object.setProperty(DIDProperty::kIconUnderlay, reader.readPackedInt());
    }

    if(flags & kMaterialType)
    {
        object.setProperty(IntProperty::kMaterialType, reader.readInt());
    }

    // properties beyond here are a guess

    if(flags2 & kCooldownID)
    {
        object.setProperty(IntProperty::kSharedCooldown, reader.readInt());
    }

    if(flags2 & kCooldownDuration)
    {
        object.setProperty(FloatProperty::kCooldownDuration, reader.readDouble());
    }

    if(flags2 & kPetOwner)
    {
        object.setProperty(IIDProperty::kPetOwner, reader.readInt());
    }

    reader.align();

    assert(reader.remaining() == 0);
}

void handleCreateObject(BinReader& reader)
{
    ObjectId objectId = ObjectId{reader.readInt()};

    Object& object = Core::get().objectManager()[objectId];

    handleObjDesc(reader, object);
    handlePhysicsDesc(reader, object);
    handleWeenieDesc(reader, object);
}
