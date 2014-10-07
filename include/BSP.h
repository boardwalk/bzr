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

struct BSPTreeType
{
    enum Value
    {
        kDrawing = 0,
        kPhysics = 1,
        kCell = 2
    };
};

class BSPNode
{
public:
    BSPNode(BinReader& reader, BSPTreeType::Value treeType, uint32_t nodeType);
    virtual ~BSPNode() {}

protected:
    BSPNode();

    Sphere bounds_;
    Plane partition_;
    unique_ptr<BSPNode> frontChild_; // may be null
    unique_ptr<BSPNode> backChild_; // may be null
    vector<uint16_t> triangleIndices_;
};

class BSPLeaf : public BSPNode
{
public:
    BSPLeaf(BinReader& reader, BSPTreeType::Value treeType);

private:
    uint32_t index_;
    uint32_t solid_;
};

class BSPPortal : public BSPNode
{
public:
    BSPPortal(BinReader& reader, BSPTreeType::Value treeType);

private:
    struct PortalPoly
    {
        uint16_t portalIndex;
        uint16_t polygonIndex;
    };

    vector<PortalPoly> portalPolys_;
};

unique_ptr<BSPNode> readBSP(BinReader& reader, BSPTreeType::Value treeType);

#endif
