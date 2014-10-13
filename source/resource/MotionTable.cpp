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
#include "resource/MotionTable.h"
#include "BinReader.h"

enum class CombatStyle : uint32_t
{
    kUndef = 0x0000,
    kUnarmed = 0x0001,
    kOneHanded = 0x0002,
    kOneHandedAndShield = 0x0004,
    kTwoHanded = 0x0008,
    kBow = 0x0010,
    kCrossbow = 0x0020,
    kSling = 0x0040,
    kThrownWeapon = 0x0080,
    kDualWield = 0x0100,
    kMagic = 0x0200,
    kAtlatl = 0x0400,
    kThrownShield = 0x0800,
    kReserved1 = 0x1000,
    kReserved2 = 0x2000,
    kReserved3 = 0x4000,
    kReserved4 = 0x8000,
    kStubbornMagic = 0x00010000,
    kStubbornProjectile = 0x00020000,
    kStubbornMelee = 0x00040000,
    kStubbornMissile = 0x00080000,
    kAllMelee = 0x010f,
    kAllMissile = 0x0cf0,
    kAllMagic = 0x0200,
    kAll = 0xffff,
};

MotionTable::MotionTable(uint32_t id, const void* data, size_t size) : ResourceImpl{id}
{
    BinReader reader(data, size);

    uint32_t resourceId = reader.readInt();
    assert(resourceId == id);

    /*defaultStyle*/ reader.readInt();
    uint32_t numStyles = reader.readInt();

    for(uint32_t i = 0; i < numStyles; i++)
    {
        reader.readInt();
        reader.readInt();
    }

    uint32_t numCycles = reader.readInt();
    cycles.reserve(numCycles);

    for(uint32_t i = 0; i < numCycles; i++)
    {
        uint32_t key = reader.readInt();
        assert(cycles.find(key) == cycles.end());
        read(reader, cycles[key]);
    }

    uint32_t numModifiers = reader.readInt();
    modifiers.reserve(numModifiers);

    for(uint32_t i = 0; i < numModifiers; i++)
    {
        uint32_t key = reader.readInt();
        assert(modifiers.find(key) == modifiers.end());
        read(reader, modifiers[key]);
    }

    uint32_t numLinks = reader.readInt();
    links.reserve(numLinks);

    for(uint32_t i = 0; i < numLinks; i++)
    {
        uint32_t outerKey = reader.readInt();
        assert(links.find(outerKey) == links.end());
        auto& innerLinks = links[outerKey];

        uint32_t numInnerLinks = reader.readInt();
        innerLinks.reserve(numInnerLinks);

        for(uint32_t j = 0; j < numInnerLinks; j++)
        {
            uint32_t innerKey = reader.readInt();
            assert(innerLinks.find(innerKey) == innerLinks.end());
            read(reader, innerLinks[innerKey]);
        }
    }

    assert(reader.remaining() == 0);
}
