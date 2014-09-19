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
#ifndef BZR_SKILLTABLE_H
#define BZR_SKILLTABLE_H

#include <map>

struct SkillType
{
    enum Value
    {
        kPhysical = 1,
        kUtility = 2,
        kMagic = 3
    };
};

struct AttributeType
{
    enum Value
    {
        kStrength = 1,
        kEndurance = 2,
        kQuickness = 3,
        kCoordination = 4,
        kFocus = 5,
        kSelf = 6
    };
};

class SkillTable
{
public:
    struct Skill
    {
        string name;
        string description;
        uint32_t icon;
        uint32_t trainCost;
        uint32_t specCost;
        SkillType::Value type;
        bool usableUntrained;
        uint32_t attribDivisor;
        AttributeType::Value attrib1;
        AttributeType::Value attrib2;
    };

    typedef map<uint32_t, Skill> container;

    SkillTable();

    container::const_iterator begin() const;
    container::const_iterator end() const;
    container::const_iterator find(uint32_t id) const;

private:
    container skills_;
};

#endif