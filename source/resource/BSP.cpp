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
#include "resource/BSP.h"
#include "physics/Sphere.h"
#include "BinReader.h"

BSPNode::BSPNode()
{}

BSPNode::BSPNode(BinReader& reader, BSPTreeType treeType, uint32_t nodeType)
{
    read(reader, partition_);

    if(nodeType == 0x42506e6e || nodeType == 0x4250496e) // BPnn, BPIn
    {
        read(reader, frontChild_, treeType);
    }
    else if(nodeType == 0x4270494e || nodeType == 0x42706e4e) // BpIN, BpnN
    {
        read(reader, backChild_ , treeType);
    }
    else if(nodeType == 0x4250494e || nodeType == 0x42506e4e) // BPIN, BPnN
    {
        read(reader, frontChild_, treeType);
        read(reader, backChild_, treeType);
    }

    if(treeType == BSPTreeType::kDrawing || treeType == BSPTreeType::kPhysics)
    {
        read(reader, bounds_);
    }

    if(treeType == BSPTreeType::kDrawing)
    {
        uint32_t triCount = reader.readInt();
        triangleIndices_.resize(triCount);

        for(uint16_t& index : triangleIndices_)
        {
            index = reader.readShort();
        }
    }
}

BSPLeaf::BSPLeaf(BinReader& reader, BSPTreeType treeType)
{
    index_ = reader.readInt();

    if(treeType == BSPTreeType::kPhysics)
    {
        // if 1, sphere parameters are valid and there are indices
        solid_ = reader.readInt();

        read(reader, bounds_);

        uint32_t triCount = reader.readInt();
        triangleIndices_.resize(triCount);

        for(uint16_t& index : triangleIndices_)
        {
            index = reader.readShort();
        }
    }
}

BSPPortal::BSPPortal(BinReader& reader, BSPTreeType treeType)
{
    read(reader, partition_);
    read(reader, frontChild_, treeType);
    read(reader, backChild_, treeType);

    if(treeType == BSPTreeType::kDrawing)
    {
        read(reader, bounds_);

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
}

void read(BinReader& reader, unique_ptr<BSPNode>& node, BSPTreeType treeType)
{
    uint32_t nodeType = reader.readInt();

    if(nodeType == 0x4c454146) // LEAF
    {
        node.reset(new BSPLeaf{reader, treeType});
    }
    else if(nodeType == 0x504f5254) // PORT
    {
        node.reset(new BSPPortal{reader, treeType});
    }
    else
    {
        node.reset(new BSPNode{reader, treeType, nodeType});
    }
}
