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
#ifndef BZR_PROPERTY_H
#define BZR_PROPERTY_H

#define ENTRY(key, value) k##key = value,

enum class BoolProperty : uint32_t
{
#include "properties/BoolProperty.inc"
};

enum class StringProperty : uint32_t
{
#include "properties/StringProperty.inc"
};

enum class IntProperty : uint32_t
{
#include "properties/IntProperty.inc"
};

enum class Int64Property : uint32_t
{
#include "properties/Int64Property.inc"
};

enum class FloatProperty : uint32_t
{
#include "properties/FloatProperty.inc"
};

enum class PositionProperty : uint32_t
{
#include "properties/PositionProperty.inc"
};

enum class IIDProperty : uint32_t
{
#include "properties/IIDProperty.inc"
};

enum class DIDProperty : uint32_t
{
#include "properties/DIDProperty.inc"
};

enum class SkillProperty : uint32_t
{
#include "properties/SkillProperty.inc"
};

enum class AttributeProperty : uint32_t
{
#include "properties/AttributeProperty.inc"
};

enum class Attribute2ndProperty : uint32_t
{
#include "properties/Attribute2ndProperty.inc"
};

#undef ENTRY

#define DECLARE_GET(t) \
  const string& get##t##PropertyName(t##Property property);

DECLARE_GET(Bool)
DECLARE_GET(String)
DECLARE_GET(Int)
DECLARE_GET(Int64)
DECLARE_GET(Float)
DECLARE_GET(Position)
DECLARE_GET(IID)
DECLARE_GET(DID)
DECLARE_GET(Skill)
DECLARE_GET(Attribute)
DECLARE_GET(Attribute2nd)

#undef DECLARE_GET

#endif