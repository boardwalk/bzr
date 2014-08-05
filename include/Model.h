#ifndef BZR_MODEL_H
#define BZR_MODEL_H

#include "math/Vec2.h"
#include "math/Vec3.h"
#include "Destructable.h"
#include "Resource.h"
#include <vector>

class Model : public ResourceImpl<Resource::Model>
{
public:
    struct Vertex
    {
        Vec3 position;
        Vec3 normal;
        vector<Vec2> texCoords;
    };

    struct Primitive
    {
        vector<int> vertexIndices;
        vector<int> texCoordIndices;
    };

    Model(const void* data, size_t size);

    const vector<uint32_t>& textures() const;
    const vector<Vertex>& vertices() const;
    const vector<Primitive>& primitives() const;

    unique_ptr<Destructable>& renderData();

private:
    vector<uint32_t> _textures;
    vector<Vertex> _vertices;
    vector<Primitive> _primitives;

    unique_ptr<Destructable> _renderData;
};

#endif