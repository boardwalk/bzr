#ifndef BZR_MODELGROUP_H
#define BZR_MODELGROUP_H

#include "math/Quat.h"
#include "math/Vec3.h"
#include "Resource.h"
#include <vector>

class ModelGroup : public ResourceImpl<Resource::ModelGroup>
{
public:
    struct Orientation
    {
        Vec3 position;
        Quat rotation;
    };

    ModelGroup(const void* data, size_t size);

    const vector<ResourcePtr>& models() const;
    const vector<uint32_t>& parents() const;
    const vector<Orientation>& orientations() const;

private:
    vector<ResourcePtr> _models;
    vector<uint32_t> _parents;
    vector<Orientation> _orientations;
};

#endif