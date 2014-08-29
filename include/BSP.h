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
#ifndef BZR_BSP_H
#define BZR_BSP_H

#include <physics/Plane.h>
#include <physics/Sphere.h>

class BinReader;
struct LineSegment;

class BSPNode
{
public:
    virtual ~BSPNode() {}

    // Collide a line segment against the BSP tree
    // If the segment contacts a solid volume, returns true and the point of impact
    virtual bool collide(const LineSegment& segment, glm::vec3& impact) const = 0;
};

class BSPInternal : public BSPNode
{
public:
    BSPInternal(BinReader& reader, int treeType, uint32_t nodeType);
    bool collide(const LineSegment& segment, glm::vec3& impact) const override;

private:
    Plane _partition;
    unique_ptr<BSPNode> _frontChild; // may be null
    unique_ptr<BSPNode> _backChild; // may be null
    // if treeType == 0 or 1
    Sphere _bounds;
    // if treeType == 0
    vector<uint16_t> _triangleIndices;
};

class BSPExternal : public BSPNode
{
public:
    BSPExternal(BinReader& reader, int treeType);
    bool collide(const LineSegment& segment, glm::vec3& impact) const override;

private:
    uint32_t _index;
    // if treeType == 1
    uint32_t _solid;
    Sphere _bounds;
    vector<uint16_t> _triangleIndices;
};

class BSPPortal : public BSPNode
{
public:
    BSPPortal(BinReader& reader, int treeType);
    bool collide(const LineSegment& segment, glm::vec3& impact) const override;

private:
    struct PortalPoly
    {
        uint16_t index;
        uint16_t what;
    };

    Plane _partition;
    unique_ptr<BSPNode> _frontChild;
    unique_ptr<BSPNode> _backChild;
    // if treeType == 0
    Sphere _bounds;
    vector<uint16_t> _triangleIndices;
    vector<PortalPoly> _portalPolys;
};

unique_ptr<BSPNode> readBSP(BinReader& reader, int treeType);

#endif
