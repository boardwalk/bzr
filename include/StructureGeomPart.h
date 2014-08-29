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
#ifndef BZR_STRUCTUREGEOMPART_H
#define BZR_STRUCTUREGEOMPART_H

#include "TriangleFan.h"
#include "Vertex.h"

class BinReader;
class BSPNode;

class StructureGeomPart
{
public:
    StructureGeomPart();
    StructureGeomPart(StructureGeomPart&&);
    ~StructureGeomPart();
    StructureGeomPart& operator=(StructureGeomPart&&);

    void read(BinReader& reader);

    const vector<Vertex>& vertices() const;
    const vector<TriangleFan>& triangleFans() const;
    const vector<TriangleFan>& hitTriangleFans() const;
    const BSPNode* hitTree() const;

private:
    vector<Vertex> _vertices;
    vector<TriangleFan> _triangleFans;
    vector<TriangleFan> _hitTriangleFans;
    unique_ptr<BSPNode> _hitTree;
};

#endif