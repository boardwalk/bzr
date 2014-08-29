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
#ifndef BZR_PHYSICS_PHYSICSOBJECT_H
#define BZR_PHYSICS_PHYSICSOBJECT_H

#include "physics/AABB.h"
#include "physics/Cylinder.h"
#include "Noncopyable.h"

class BSPNode;
class Landblock;

struct ShapeType
{
    enum Value
    {
        Unknown,
        BSPTree,
        Landblock,
        Cylinder
    };
};

class PhysicsObject : Noncopyable
{
public:
    PhysicsObject();

    ShapeType::Value type() const;

    void setBSPTree(const BSPNode* tree);
    const BSPNode* getBSPTree() const;

    void setLandblock(const Landblock* landblock);
    const Landblock* getLandblock() const;

    void setCylinder(const Cylinder& cylinder);
    const Cylinder& getCylinder() const;

    void setOffset(const glm::vec3& offset);
    const glm::vec3& getOffset() const;

private:
    union Data
    {
        // when _type == BSPTree
        const BSPNode* tree;
        // when _type == Landblock
        const Landblock* landblock;
        // when _type == Cylinder
        Cylinder cylinder;
    };

    ShapeType::Value _type;
    Data _data;
    glm::vec3 _offset;
    AABB _bounds;
};

#endif
