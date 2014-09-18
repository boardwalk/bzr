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

    virtual const Sphere& bounds() const = 0;
};

class BSPInternal : public BSPNode
{
public:
    BSPInternal(BinReader& reader, int treeType, uint32_t nodeType);

    bool collide(const LineSegment& segment, glm::vec3& impact) const override;

    const Sphere& bounds() const override;

private:
    Plane partition_;
    unique_ptr<BSPNode> frontChild_; // may be null
    unique_ptr<BSPNode> backChild_; // may be null
    // if treeType == 0 or 1
    Sphere bounds_;
    // if treeType == 0
    vector<uint16_t> triangleIndices_;
};

class BSPExternal : public BSPNode
{
public:
    BSPExternal(BinReader& reader, int treeType);

    bool collide(const LineSegment& segment, glm::vec3& impact) const override;

    const Sphere& bounds() const override;

private:
    uint32_t index_;
    // if treeType == 1
    uint32_t solid_;
    Sphere bounds_;
    vector<uint16_t> triangleIndices_;
};

class BSPPortal : public BSPNode
{
public:
    BSPPortal(BinReader& reader, int treeType);

    bool collide(const LineSegment& segment, glm::vec3& impact) const override;

    const Sphere& bounds() const override;

private:
    struct PortalPoly
    {
        uint16_t index;
        uint16_t what;
    };

    Plane partition_;
    unique_ptr<BSPNode> frontChild_;
    unique_ptr<BSPNode> backChild_;
    // if treeType == 0
    Sphere bounds_;
    vector<uint16_t> triangleIndices_;
    vector<PortalPoly> portalPolys_;
};

unique_ptr<BSPNode> readBSP(BinReader& reader, int treeType);

#endif
