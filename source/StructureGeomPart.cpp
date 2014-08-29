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
#include "StructureGeomPart.h"
#include "BinReader.h"
#include "BSP.h"

StructureGeomPart::StructureGeomPart()
{}

StructureGeomPart::StructureGeomPart(StructureGeomPart&& other)
{
    _vertices = move(other._vertices);
    _triangleFans = move(other._triangleFans);
    _hitTriangleFans = move(other._hitTriangleFans);
    _hitTree = move(other._hitTree);
}

StructureGeomPart::~StructureGeomPart()
{}

StructureGeomPart& StructureGeomPart::operator=(StructureGeomPart&& other)
{
    _vertices = move(other._vertices);
    _triangleFans = move(other._triangleFans);
    _hitTriangleFans = move(other._hitTriangleFans);
    _hitTree = move(other._hitTree);
    return *this;
}

void StructureGeomPart::read(BinReader& reader)
{
    auto numTriangleFans = reader.read<uint32_t>();
    _triangleFans.resize(numTriangleFans);

    auto numHitTriangleFans = reader.read<uint32_t>();
    _hitTriangleFans.resize(numHitTriangleFans);

    auto numShorts = reader.read<uint32_t>();

    auto unk5 = reader.read<uint32_t>();
    assert(unk5 == 1);

    auto numVertices = reader.read<uint32_t>();
    _vertices.resize(numVertices);

    for(auto vi = 0u; vi < numVertices; vi++)
    {
        auto vertexNum = reader.read<uint16_t>();
        assert(vertexNum == vi);

        _vertices[vi].read(reader);
    }

    for(auto tfi = 0u; tfi < numTriangleFans; tfi++)
    {
        auto triangleFanNum = reader.read<uint16_t>();
        assert(triangleFanNum == tfi);

        _triangleFans[tfi].read(reader);
    }

    for(auto si = 0u; si < numShorts; si++)
    {
        reader.read<uint16_t>();
    }
    reader.align();

    readBSP(reader, 2);

    for(auto htfi = 0u; htfi < numHitTriangleFans; htfi++)
    {
        auto triangleFanNum = reader.read<uint16_t>();
        assert(triangleFanNum == htfi);

        _hitTriangleFans[htfi].read(reader);
    }

    readBSP(reader, 1);

    auto unk7 = reader.read<uint32_t>();
    assert(unk7 == 0 || unk7 == 1);

    if(unk7)
    {
        readBSP(reader, 0);
    }

    reader.align();
}

const vector<Vertex>& StructureGeomPart::vertices() const
{
    return _vertices;
}

const vector<TriangleFan>& StructureGeomPart::triangleFans() const
{
    return _triangleFans;
}

const vector<TriangleFan>& StructureGeomPart::hitTriangleFans() const
{
    return _hitTriangleFans;
}

const BSPNode* StructureGeomPart::hitTree() const
{
    return _hitTree.get();
}
