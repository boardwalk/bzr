#ifndef BZR_MODELGROUP_H
#define BZR_MODELGROUP_H

#include "math/Quat.h"
#include "math/Vec3.h"
#include "Resource.h"
#include <vector>

class ModelGroup : public ResourceImpl<Resource::ModelGroup>
{
public:
    struct ModelInfo
    {
        ModelInfo() : parent(0xFFFFFFFF)
        {}

        ResourcePtr resource;
        uint32_t parent;
        Vec3 position;
        Quat rotation;
    };

    ModelGroup(const void* data, size_t size);

    const vector<ModelInfo>& modelInfos() const;

private:
    vector<ModelInfo> _modelInfos;
};

#endif