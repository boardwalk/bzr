#ifndef BZR_STRUCTUREGEOM_H
#define BZR_STRUCTUREGEOM_H

#include "Resource.h"
#include "TriangleFan.h"
#include "Vertex.h"
#include <vector>

class StructureGeom : public ResourceImpl<ResourceType::StructureGeom>
{
public:
    StructureGeom(uint32_t id, const void* data, size_t size);

    const vector<Vertex>& vertices() const;
    const vector<TriangleFan>& triangleFans() const;

private:
    vector<Vertex> _vertices;
    vector<TriangleFan> _triangleFans;
};

#endif