/*
 * Bael'Zharon's Respite
 * Copyright (C) 2014 Daniel Skorupski
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; withuot even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */
#include "Property.h"
#include <unordered_map>

#define ENTRY(key, value) {value, #key},

static const unordered_map<uint32_t, string> kBoolProperties
{
#include "properties/BoolProperty.inc"
};

static const unordered_map<uint32_t, string> kStringProperties
{
#include "properties/StringProperty.inc"
};

static const unordered_map<uint32_t, string> kIntProperties
{
#include "properties/IntProperty.inc"
};

static const unordered_map<uint32_t, string> kInt64Properties
{
#include "properties/Int64Property.inc"
};

static const unordered_map<uint32_t, string> kFloatProperties
{
#include "properties/FloatProperty.inc"
};

static const unordered_map<uint32_t, string> kPositionProperties
{
#include "properties/PositionProperty.inc"
};

static const unordered_map<uint32_t, string> kIIDProperties
{
#include "properties/IIDProperty.inc"
};

static const unordered_map<uint32_t, string> kDIDProperties
{
#include "properties/DIDProperty.inc"
};

static const unordered_map<uint32_t, string> kSkillProperties
{
#include "properties/SkillProperty.inc"
};

static const unordered_map<uint32_t, string> kAttributeProperties
{
#include "properties/AttributeProperty.inc"
};

static const unordered_map<uint32_t, string> kAttribute2ndProperties
{
#include "properties/Attribute2ndProperty.inc"
};

static const string kUnknown = "(unknown)";

#define IMPLEMENT_GET(t) \
    const string& get##t##PropertyName(t##Property property) \
    { \
        auto it = k##t##Properties.find(static_cast<uint32_t>(property)); \
        if(it == k##t##Properties.end()) \
        { \
            return kUnknown; \
        } \
        return it->second; \
    }

IMPLEMENT_GET(Bool);
IMPLEMENT_GET(String);
IMPLEMENT_GET(Int);
IMPLEMENT_GET(Int64);
IMPLEMENT_GET(Float);
IMPLEMENT_GET(Position);
IMPLEMENT_GET(IID);
IMPLEMENT_GET(DID);
IMPLEMENT_GET(Skill);
IMPLEMENT_GET(Attribute);
IMPLEMENT_GET(Attribute2nd);
