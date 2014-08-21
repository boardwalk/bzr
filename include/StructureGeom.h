#ifndef BZR_STRUCTUREGEOM_H
#define BZR_STRUCTUREGEOM_H

#include "Resource.h"
#include "TriangleFan.h"
#include "Vertex.h"

class StructureGeom : public ResourceImpl<ResourceType::StructureGeom>
{
public:
    struct Piece
    {
        vector<Vertex> vertices;
        vector<TriangleFan> triangleFans;
    };

    StructureGeom(uint32_t id, const void* data, size_t size);

    const vector<Piece>& pieces() const;

private:
    vector<Piece> _pieces;
};

#endif