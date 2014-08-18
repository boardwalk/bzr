#ifndef BZR_VERTEX_H
#define BZR_VERTEX_H

#include "math/Vec2.h"
#include "math/Vec3.h"
#include <vector>

class BlobReader;

struct Vertex
{
    Vec3 position;
    Vec3 normal;
    vector<Vec2> texCoords;

    void read(BlobReader& reader);
};

#endif