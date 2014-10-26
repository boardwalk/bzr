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
#ifndef BZR_OBJECT_H
#define BZR_OBJECT_H

#include "Property.h"
#include "Noncopyable.h"
#include "LandcellId.h"
#include "Location.h"
#include "ObjectId.h"
#include "Resource.h"
#include <unordered_map>

struct Position
{
    LandcellId landcell;
    glm::vec3 position;
    glm::quat rotation;
};

class Object : Noncopyable
{
public:
    Object(ObjectId id);

    void setProperty(BoolProperty property, bool value);
    void setProperty(StringProperty property, string value);
    void setProperty(IntProperty property, uint32_t value);
    void setProperty(Int64Property property, uint64_t value);
    void setProperty(FloatProperty property, double value);
    void setProperty(PositionProperty property, Position value);
    void setProperty(IIDProperty property, uint32_t value);
    void setProperty(DIDProperty property, uint32_t value);
    // Skill
    // Attribute
    // Attribute2nd

    void setModel(ResourcePtr model);
    void setLandcellId(const LandcellId& landcellId);
    void setLocation(const Location& location);

    ObjectId id() const;
    const ResourcePtr& model() const;
    const LandcellId& landcellId() const;
    const Location& location() const;

private:
    const ObjectId id_;

    unordered_map<BoolProperty, bool> boolProperties_;
    unordered_map<StringProperty, string> stringProperties_;
    unordered_map<IntProperty, uint32_t> intProperties_;
    unordered_map<Int64Property, uint64_t> int64Properties_;
    unordered_map<FloatProperty, double> floatProperties_;
    unordered_map<PositionProperty, Position> positionProperties_;
    unordered_map<IIDProperty, uint32_t> iidProperties_;
    unordered_map<DIDProperty, uint32_t> didProperties_;

    ResourcePtr model_;
    LandcellId landcellId_;
    Location location_;
};

#endif
