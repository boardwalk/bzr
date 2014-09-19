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
    vertices = move(other.vertices);
    triangleFans = move(other.triangleFans);
    hitTriangleFans = move(other.hitTriangleFans);
    hitTree = move(other.hitTree);
}

StructureGeomPart::~StructureGeomPart()
{}

StructureGeomPart& StructureGeomPart::operator=(StructureGeomPart&& other)
{
    vertices = move(other.vertices);
    triangleFans = move(other.triangleFans);
    hitTriangleFans = move(other.hitTriangleFans);
    hitTree = move(other.hitTree);
    return *this;
}

void StructureGeomPart::read(BinReader& reader)
{
    uint32_t numTriangleFans = reader.read<uint32_t>();
    triangleFans.resize(numTriangleFans);

    uint32_t numHitTriangleFans = reader.read<uint32_t>();
    hitTriangleFans.resize(numHitTriangleFans);

    uint32_t numShorts = reader.read<uint32_t>();

    uint32_t unk5 = reader.read<uint32_t>();
    assert(unk5 == 1);

    uint32_t numVertices = reader.read<uint32_t>();
    vertices.resize(numVertices);

    for(uint32_t vi = 0; vi < numVertices; vi++)
    {
        uint16_t vertexNum = reader.read<uint16_t>();
        assert(vertexNum == vi);

        vertices[vi].read(reader);
    }

    for(uint32_t tfi = 0; tfi < numTriangleFans; tfi++)
    {
        uint16_t triangleFanNum = reader.read<uint16_t>();
        assert(triangleFanNum == tfi);

        triangleFans[tfi].read(reader);
    }

    for(uint32_t si = 0; si < numShorts; si++)
    {
        reader.read<uint16_t>();
    }
    reader.align();

    readBSP(reader, 2);

    for(uint32_t htfi = 0; htfi < numHitTriangleFans; htfi++)
    {
        uint16_t triangleFanNum = reader.read<uint16_t>();
        assert(triangleFanNum == htfi);

        hitTriangleFans[htfi].read(reader);
    }

    readBSP(reader, 1);

    uint32_t unk7 = reader.read<uint32_t>();
    assert(unk7 == 0 || unk7 == 1);

    if(unk7)
    {
        readBSP(reader, 0);
    }

    reader.align();
}
