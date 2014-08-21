#ifndef BZR_MODEL_H
#define BZR_MODEL_H

#include "Destructable.h"
#include "Resource.h"
#include "Vertex.h"
#include "TriangleFan.h"
#include <vector>

class Model : public ResourceImpl<ResourceType::Model>
{
public:
    Model(uint32_t id, const void* data, size_t size);

    const vector<ResourcePtr>& textures() const;
    const vector<Vertex>& vertices() const;
    const vector<TriangleFan>& triangleFans() const;

    // If true, the model has transparent or translucent elements and must be depth sorted before rendering
    bool needsDepthSort() const;

    unique_ptr<Destructable>& renderData();

private:
    vector<ResourcePtr> _textures;
    vector<Vertex> _vertices;
    vector<TriangleFan> _triangleFans;
    bool _needsDepthSort;

    unique_ptr<Destructable> _renderData;
};

#endif