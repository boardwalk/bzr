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
    vertices_ = move(other.vertices_);
    triangleFans_ = move(other.triangleFans_);
    hitTriangleFans_ = move(other.hitTriangleFans_);
    hitTree_ = move(other.hitTree_);
}

StructureGeomPart::~StructureGeomPart()
{}

StructureGeomPart& StructureGeomPart::operator=(StructureGeomPart&& other)
{
    vertices_ = move(other.vertices_);
    triangleFans_ = move(other.triangleFans_);
    hitTriangleFans_ = move(other.hitTriangleFans_);
    hitTree_ = move(other.hitTree_);
    return *this;
}

void StructureGeomPart::read(BinReader& reader)
{
    auto numTriangleFans = reader.read<uint32_t>();
    triangleFans_.resize(numTriangleFans);

    auto numHitTriangleFans = reader.read<uint32_t>();
    hitTriangleFans_.resize(numHitTriangleFans);

    auto numShorts = reader.read<uint32_t>();

    auto unk5 = reader.read<uint32_t>();
    assert(unk5 == 1);

    auto numVertices = reader.read<uint32_t>();
    vertices_.resize(numVertices);

    for(auto vi = 0u; vi < numVertices; vi++)
    {
        auto vertexNum = reader.read<uint16_t>();
        assert(vertexNum == vi);

        vertices_[vi].read(reader);
    }

    for(auto tfi = 0u; tfi < numTriangleFans; tfi++)
    {
        auto triangleFanNum = reader.read<uint16_t>();
        assert(triangleFanNum == tfi);

        triangleFans_[tfi].read(reader);
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

        hitTriangleFans_[htfi].read(reader);
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
    return vertices_;
}

const vector<TriangleFan>& StructureGeomPart::triangleFans() const
{
    return triangleFans_;
}

const vector<TriangleFan>& StructureGeomPart::hitTriangleFans() const
{
    return hitTriangleFans_;
}

const BSPNode* StructureGeomPart::hitTree() const
{
    return hitTree_.get();
}
