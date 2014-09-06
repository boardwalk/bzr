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
#ifndef BZR_PHYSICS_BODY_H
#define BZR_PHYSICS_BODY_H

#include "physics/AABB.h"
#include "physics/Cylinder.h"
#include "Location.h"
#include "Noncopyable.h"
#include "ilist.h"

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

struct BodyFlags
{
    enum Value
    {
        // This body never moves
        Static = 0x1,
        // This body collides with other objects, but other objects pass through it
        Ghost = 0x2,
        // This body is effected by gravity
        HasGravity = 0x4
    };
};

struct BeginXTag;
struct BeginYTag;
struct EndXTag;
struct EndYTag;
struct ActiveTag;

class Body : Noncopyable,
    public ilist_node<Body, BeginXTag>,
    public ilist_node<Body, BeginYTag>,
    public ilist_node<Body, EndXTag>,
    public ilist_node<Body, EndYTag>,
    public ilist_node<Body, ActiveTag>
{
public:
    Body();

    ShapeType::Value type() const;

    void setFlags(BodyFlags::Value flags);
    BodyFlags::Value flags() const;

    void setBSPTree(const BSPNode* tree);
    const BSPNode* getBSPTree() const;

    void setLandblock(const Landblock* landblock);
    const Landblock* getLandblock() const;

    void setCylinder(const Cylinder& cylinder);
    const Cylinder& getCylinder() const;

    const AABB& bounds() const;

    void setLocation(const Location& location);
    const Location& location() const;

    void setVelocity(const glm::vec3& velocity);
    const glm::vec3& velocity() const;

    void setTag(uint64_t tag);
    uint64_t tag() const;

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
    BodyFlags::Value _flags;
    Data _data;
    AABB _bounds;
    Location _location;
    glm::vec3 _velocity;
    uint64_t _tag;
};

#endif
