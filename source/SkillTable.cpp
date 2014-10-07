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
#include "SkillTable.h"
#include "BinReader.h"
#include "Core.h"
#include "DatFile.h"

SkillTable::SkillTable()
{
    vector<uint8_t> data = Core::get().portalDat().read(0x0e000004);

    BinReader reader(data.data(), data.size());

    uint32_t resourceId = reader.readInt();
    assert(resourceId == 0x0e000004);

    uint16_t numSkills = reader.readShort();

    uint16_t unk1 = reader.readShort();
    assert(unk1 == 0x20);

    for(uint16_t i = 0; i < numSkills; i++)
    {
        uint32_t id = reader.readInt();
        Skill& skill = skills_[id];

        skill.description = reader.readString();
        skill.name = reader.readString();

        skill.icon = reader.readInt();
        assert((skill.icon & 0xFF000000) == 0x06000000);

        skill.trainCost = reader.readInt();
        skill.specCost = reader.readInt();
        skill.category = (SkillCategory::Value)reader.readInt();

        uint32_t unk2 = reader.readInt();
        assert(unk2 == 1);

        uint32_t charGenUse = reader.readInt();
        assert(charGenUse == 1 || charGenUse == 2);
        skill.usableUntrained = (charGenUse == 1);

        uint32_t minLevel = reader.readInt();
        assert(minLevel == 0);

        uint32_t hasAttrib1 = reader.readInt();
        assert(hasAttrib1 == 0 || hasAttrib1 == 1);

        uint32_t hasAttrib2 = reader.readInt();
        assert(hasAttrib2 == 0 || hasAttrib2 == 1);

        skill.attribDivisor = reader.readInt();
        skill.attrib1 = (AttributeType::Value)reader.readInt();
        skill.attrib2 = (AttributeType::Value)reader.readInt();

        /*double upperBound = */reader.readDouble();
        /*double lowerBound = */reader.readDouble();

        double learnMod = reader.readDouble();
        assert(learnMod == 1.0);
    }

    reader.assertEnd();
}
