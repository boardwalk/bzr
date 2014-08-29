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

BSPInternal::BSPInternal(BinReader& reader, int treeType, uint32_t nodeType)
{
    _partition.normal.x = reader.read<float>();
    _partition.normal.y = reader.read<float>();
    _partition.normal.z = reader.read<float>();
    _partition.dist = reader.read<float>();

    if(nodeType == 0x42506e6e || nodeType == 0x4250496e) // BPnn, BPIn
    {
        _frontChild = readBSP(reader, treeType);
    }
    else if(nodeType == 0x4270494e || nodeType == 0x42706e4e) // BpIN, BpnN
    {
        _backChild = readBSP(reader, treeType);
    }
    else if(nodeType == 0x4250494e || nodeType == 0x42506e4e) // BPIN, BPnN
    {
        _frontChild = readBSP(reader, treeType);
        _backChild = readBSP(reader, treeType);
    }

    if(treeType == 0 || treeType == 1)
    {
        _bounds.center.x = reader.read<float>();
        _bounds.center.y = reader.read<float>();
        _bounds.center.z = reader.read<float>();
        _bounds.radius = reader.read<float>();
    }

    if(treeType != 0)
    {
        return;
    }

    auto triCount = reader.read<uint32_t>();
    _triangleIndices.resize(triCount);

    for(auto& index : _triangleIndices)
    {
        index = reader.read<uint16_t>();
    }
}

bool BSPInternal::collide(const LineSegment& segment, glm::vec3& impact) const
{
    // TODO
    (void)segment;
    (void)impact;

    return false;
}

BSPExternal::BSPExternal(BinReader& reader, int treeType)
{
    _index = reader.read<uint32_t>();

    if(treeType != 1)
    {
        return;
    }

    // if 1, sphere parameters are valid and there are indices
    _solid = reader.read<uint32_t>();

    _bounds.center.x = reader.read<float>();
    _bounds.center.y = reader.read<float>();
    _bounds.center.z = reader.read<float>();
    _bounds.radius = reader.read<float>();

    auto triCount = reader.read<uint32_t>();
    _triangleIndices.resize(triCount);

    for(auto& index : _triangleIndices)
    {
        index = reader.read<uint16_t>();
    }
}

bool BSPExternal::collide(const LineSegment& segment, glm::vec3& impact) const
{
    if(_solid)
    {
        impact = segment.begin;
        return true;
    }

    return false;
}

BSPPortal::BSPPortal(BinReader& reader, int treeType)
{
    _partition.normal.x = reader.read<float>();
    _partition.normal.y = reader.read<float>();
    _partition.normal.z = reader.read<float>();
    _partition.dist = reader.read<float>();

    _frontChild = readBSP(reader, treeType);
    _backChild = readBSP(reader, treeType);

    if(treeType != 0)
    {
        return;
    }

    _bounds.center.x = reader.read<float>();
    _bounds.center.y = reader.read<float>();
    _bounds.center.z = reader.read<float>();
    _bounds.radius = reader.read<float>();

    auto triCount = reader.read<uint32_t>();
    _triangleIndices.resize(triCount);

    auto polyCount = reader.read<uint32_t>();
    _portalPolys.resize(polyCount);

    for(auto& index : _triangleIndices)
    {
        index = reader.read<uint16_t>();
    }

    for(auto& poly : _portalPolys)
    {
        poly.index = reader.read<uint16_t>();
        poly.what = reader.read<uint16_t>();
    }
}

bool BSPPortal::collide(const LineSegment& segment, glm::vec3& impact) const
{
    // TODO
    (void)segment;
    (void)impact;
    
    return false;
}

unique_ptr<BSPNode> readBSP(BinReader& reader, int treeType)
{
    auto nodeType = reader.read<uint32_t>();

    if(nodeType == 0x4c454146) // LEAF
    {
        return unique_ptr<BSPNode>(new BSPExternal(reader, treeType));
    }
    else if(nodeType == 0x504f5254) // PORT
    {
        return unique_ptr<BSPNode>(new BSPPortal(reader, treeType));
    }
    else
    {
        return unique_ptr<BSPNode>(new BSPInternal(reader, treeType, nodeType));
    }
}
