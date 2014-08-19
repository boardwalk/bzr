#ifndef BZR_STRUCTUREGEOM_H
#define BZR_STRUCTUREGEOM_H

#include "Resource.h"
#include "TriangleStrip.h"
#include "Vertex.h"
#include <vector>

class StructureGeom : public ResourceImpl<ResourceType::StructureGeom>
{
public:
    StructureGeom(uint32_t id, const void* data, size_t size);

    const vector<Vertex>& vertices() const;
    const vector<TriangleStrip>& triangleStrips() const;

private:
    vector<Vertex> _vertices;
    vector<TriangleStrip> _triangleStrips;
};

#endif