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
#include "BSP.h"
#include <physics/LineSegment.h>
#include "BinReader.h"

BSPNode::BSPNode()
{}

BSPNode::BSPNode(BinReader& reader, int treeType, uint32_t nodeType)
{
    partition_.normal.x = reader.readFloat();
    partition_.normal.y = reader.readFloat();
    partition_.normal.z = reader.readFloat();
    partition_.dist = reader.readFloat();

    if(nodeType == 0x42506e6e || nodeType == 0x4250496e) // BPnn, BPIn
    {
        frontChild_ = readBSP(reader, treeType);
    }
    else if(nodeType == 0x4270494e || nodeType == 0x42706e4e) // BpIN, BpnN
    {
        backChild_ = readBSP(reader, treeType);
    }
    else if(nodeType == 0x4250494e || nodeType == 0x42506e4e) // BPIN, BPnN
    {
        frontChild_ = readBSP(reader, treeType);
        backChild_ = readBSP(reader, treeType);
    }

    if(treeType == 0 || treeType == 1)
    {
        bounds_.center.x = reader.readFloat();
        bounds_.center.y = reader.readFloat();
        bounds_.center.z = reader.readFloat();
        bounds_.radius = reader.readFloat();
    }

    if(treeType != 0)
    {
        return;
    }

    uint32_t triCount = reader.readInt();
    triangleIndices_.resize(triCount);

    for(uint16_t& index : triangleIndices_)
    {
        index = reader.readShort();
    }
}

BSPLeaf::BSPLeaf(BinReader& reader, int treeType)
{
    index_ = reader.readInt();

    if(treeType != 1)
    {
        return;
    }

    // if 1, sphere parameters are valid and there are indices
    solid_ = reader.readInt();

    bounds_.center.x = reader.readFloat();
    bounds_.center.y = reader.readFloat();
    bounds_.center.z = reader.readFloat();
    bounds_.radius = reader.readFloat();

    uint32_t triCount = reader.readInt();
    triangleIndices_.resize(triCount);

    for(uint16_t& index : triangleIndices_)
    {
        index = reader.readShort();
    }
}

BSPPortal::BSPPortal(BinReader& reader, int treeType)
{
    partition_.normal.x = reader.readFloat();
    partition_.normal.y = reader.readFloat();
    partition_.normal.z = reader.readFloat();
    partition_.dist = reader.readFloat();

    frontChild_ = readBSP(reader, treeType);
    backChild_ = readBSP(reader, treeType);

    if(treeType != 0)
    {
        return;
    }

    bounds_.center.x = reader.readFloat();
    bounds_.center.y = reader.readFloat();
    bounds_.center.z = reader.readFloat();
    bounds_.radius = reader.readFloat();

    uint32_t triCount = reader.readInt();
    triangleIndices_.resize(triCount);

    uint32_t polyCount = reader.readInt();
    portalPolys_.resize(polyCount);

    for(uint16_t& index : triangleIndices_)
    {
        index = reader.readShort();
    }

    for(PortalPoly& poly : portalPolys_)
    {
        poly.portalIndex = reader.readShort();
        poly.polygonIndex = reader.readShort();
    }
}

unique_ptr<BSPNode> readBSP(BinReader& reader, int treeType)
{
    uint32_t nodeType = reader.readInt();

    if(nodeType == 0x4c454146) // LEAF
    {
        return unique_ptr<BSPNode>(new BSPLeaf{reader, treeType});
    }
    else if(nodeType == 0x504f5254) // PORT
    {
        return unique_ptr<BSPNode>(new BSPPortal{reader, treeType});
    }
    else
    {
        return unique_ptr<BSPNode>(new BSPNode{reader, treeType, nodeType});
    }
}
