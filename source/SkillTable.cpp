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

    uint32_t resourceId = reader.read<uint32_t>();
    assert(resourceId == 0x0e000004);

    uint16_t numSkills = reader.read<uint16_t>();

    uint16_t unk1 = reader.read<uint16_t>();
    assert(unk1 == 0x20);

    for(uint16_t si = 0; si < numSkills; si++)
    {
        uint32_t id = reader.read<uint32_t>();
        Skill& skill = skills_[id];

        skill.description = reader.readString();
        skill.name = reader.readString();

        skill.icon = reader.read<uint32_t>();
        assert((skill.icon & 0xFF000000) == 0x06000000);

        skill.trainCost = reader.read<uint32_t>();
        skill.specCost = reader.read<uint32_t>();
        skill.type = (SkillType::Value)reader.read<uint32_t>();

        uint32_t unk2 = reader.read<uint32_t>();
        assert(unk2 == 1);

        uint32_t usableUntrained = reader.read<uint32_t>();
        assert(usableUntrained == 1 || usableUntrained == 2);
        skill.usableUntrained = (usableUntrained == 1);

        uint32_t unk3 = reader.read<uint32_t>();
        assert(unk3 == 0);

        uint32_t hasAttrib1 = reader.read<uint32_t>();
        assert(hasAttrib1 == 0 || hasAttrib1 == 1);

        uint32_t hasAttrib2 = reader.read<uint32_t>();
        assert(hasAttrib2 == 0 || hasAttrib2 == 1);

        skill.attribDivisor = reader.read<uint32_t>();
        skill.attrib1 = (AttributeType::Value)reader.read<uint32_t>();
        skill.attrib2 = (AttributeType::Value)reader.read<uint32_t>();

        reader.read<double>();
        reader.read<double>();

        double unk4 = reader.read<double>();
        assert(unk4 == 1.0);
    }

    reader.assertEnd();
}
