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

    struct Index
    {
        Index() : vertexIndex(0), texCoordIndex(0)
        {}

        int vertexIndex;
        int texCoordIndex;
    };

    struct Primitive
    {
        Primitive() : texIndex(0)
        {}

        int texIndex;
        vector<Index> indices;
    };

    Model(uint32_t id, const void* data, size_t size);

    const vector<ResourcePtr>& textures() const;
    const vector<Vertex>& vertices() const;
    const vector<Primitive>& primitives() const;

    unique_ptr<Destructable>& renderData();

private:
    vector<ResourcePtr> _textures;
    vector<Vertex> _vertices;
    vector<Primitive> _primitives;

    unique_ptr<Destructable> _renderData;
};

#endif