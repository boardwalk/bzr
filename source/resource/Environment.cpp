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
#include "resource/Environment.h"
#include "resource/BSP.h"
#include "BinReader.h"
#include "TriangleFan.h"
#include "Vertex.h"

CellStruct::CellStruct()
{}

CellStruct::CellStruct(CellStruct&& other)
{
    vertices = move(other.vertices);
    triangleFans = move(other.triangleFans);
    hitTriangleFans = move(other.hitTriangleFans);
    hitTree = move(other.hitTree);
}

CellStruct::~CellStruct()
{}

CellStruct& CellStruct::operator=(CellStruct&& other)
{
    vertices = move(other.vertices);
    triangleFans = move(other.triangleFans);
    hitTriangleFans = move(other.hitTriangleFans);
    hitTree = move(other.hitTree);
    return *this;
}

static void read(BinReader& reader, CellStruct& part)
{
    uint32_t numTriangleFans = reader.readInt();
    part.triangleFans.resize(numTriangleFans);

    uint32_t numHitTriangleFans = reader.readInt();
    part.hitTriangleFans.resize(numHitTriangleFans);

    uint32_t numPortals = reader.readInt();

    uint32_t unk5 = reader.readInt();
    assert(unk5 == 1);

    uint32_t numVertices = reader.readInt();
    part.vertices.resize(numVertices);

    for(uint32_t i = 0; i < numVertices; i++)
    {
        uint16_t vertexNum = reader.readShort();
        assert(vertexNum == i);

        read(reader, part.vertices[i]);
    }

    for(uint32_t i = 0; i < numTriangleFans; i++)
    {
        uint16_t triangleFanNum = reader.readShort();
        assert(triangleFanNum == i);

        read(reader, part.triangleFans[i]);
    }

    for(uint32_t i = 0; i < numPortals; i++)
    {
        reader.readShort();
    }
    reader.align();

    unique_ptr<BSPNode> cellBSP;
    read(reader, cellBSP, BSPTreeType::kCell);

    for(uint32_t i = 0; i < numHitTriangleFans; i++)
    {
        uint16_t triangleFanNum = reader.readShort();
        assert(triangleFanNum == i);

        read(reader, part.hitTriangleFans[i]);
    }
    
    unique_ptr<BSPNode> physicsBSP;
    read(reader, physicsBSP, BSPTreeType::kPhysics);

    uint32_t hasDrawingBSP = reader.readInt();
    assert(hasDrawingBSP == 0 || hasDrawingBSP == 1);

    if(hasDrawingBSP)
    {
        unique_ptr<BSPNode> drawingBSP;
        read(reader, drawingBSP, BSPTreeType::kDrawing);
    }

    reader.align();
}

Environment::Environment(uint32_t id, const void* data, size_t size) : ResourceImpl{id}
{
    BinReader reader(data, size);

    uint32_t resourceId = reader.readInt();
    assert(resourceId == id);

    uint32_t numParts = reader.readInt();
    parts.resize(numParts);

    for(uint32_t i = 0; i < numParts; i++)
    {
        uint32_t partNum = reader.readInt();
        assert(partNum == i);

        read(reader, parts[i]);
    }

    assert(reader.remaining() == 0);
}
