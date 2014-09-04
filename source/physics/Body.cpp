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
#include "physics/Body.h"
#include "BSP.h"

Body::Body() : _type(ShapeType::Unknown)
{}

ShapeType::Value Body::type() const
{
    return _type;
}

void Body::setFlags(BodyFlags::Value flags)
{
    _flags = flags;
}

BodyFlags::Value Body::flags() const
{
    return _flags;
}

void Body::setBSPTree(const BSPNode* tree)
{
    _type = ShapeType::BSPTree;
    _data.tree = tree;
    auto sphereBounds = tree->bounds();
    _bounds.min = sphereBounds.center - glm::vec3(sphereBounds.radius);
    _bounds.max = sphereBounds.center + glm::vec3(sphereBounds.radius);
}

const BSPNode* Body::getBSPTree() const
{
    assert(_type == ShapeType::BSPTree);
    return _data.tree;
}

void Body::setLandblock(const Landblock* landblock)
{
    _type = ShapeType::Landblock;
    _data.landblock = landblock;
    _bounds.min = glm::vec3(0.0, 0.0, 0.0);
    _bounds.max = glm::vec3(192.0, 192.0, 512.0);
}

const Landblock* Body::getLandblock() const
{
    assert(_type == ShapeType::Landblock);
    return _data.landblock;
}

void Body::setCylinder(const Cylinder& cylinder)
{
    _type = ShapeType::Cylinder;
    _data.cylinder = cylinder;
    _bounds.min = glm::vec3(-cylinder.radius, -cylinder.radius, -cylinder.halfHeight);
    _bounds.max = glm::vec3(cylinder.radius, cylinder.radius, cylinder.halfHeight);
}

const Cylinder& Body::getCylinder() const
{
    assert(_type == ShapeType::Cylinder);
    return _data.cylinder;
}

void Body::setLocation(const Location& location)
{
    _location = location;
}

const Location& Body::location() const
{
    return _location;
}

void Body::setVelocity(const glm::vec3& velocity)
{
    _velocity = velocity;
}

const glm::vec3& Body::velocity() const
{
    return _velocity;
}

void Body::setTag(uint64_t tag)
{
    _tag = tag;
}

uint64_t Body::tag() const
{
    return _tag;
}
