#include "ModelGroup.h"
#include "BlobReader.h"
#include "Core.h"
#include "ResourceCache.h"

ModelGroup::ModelGroup(const void* data, size_t size)
{
    BlobReader reader(data, size);

    auto fileId = reader.read<uint32_t>();
    assert((fileId & 0xFF000000) == 0x02000000);

    auto flags = reader.read<uint32_t>();
    assert(flags <= 0xF);

    auto modelCount = reader.read<uint32_t>();
    _models.resize(modelCount);
    _parents.resize(modelCount, 0xFFFFFFFF);
    _orientations.resize(modelCount);

    for(auto& model : _models)
    {
        auto modelId = reader.read<uint32_t>();
        model = Core::get().resourceCache().get(modelId);
    }

    if(flags & 1)
    {
        for(auto& parent : _parents)
        {
            parent = reader.read<uint32_t>();
        }
    }

    if(flags & 2)
    {
        for(auto i = 0u; i < modelCount; i++)
        {
            reader.read<uint32_t>();
            reader.read<uint32_t>();
            reader.read<uint32_t>();
        }
    }

    reader.read<uint32_t>();
    reader.read<uint32_t>();
    reader.read<uint32_t>();
    reader.read<uint32_t>();

    for(auto& orientation : _orientations)
    {
        orientation.position.x = reader.read<float>();
        orientation.position.y = reader.read<float>();
        orientation.position.z = reader.read<float>();

        orientation.rotation.w = reader.read<float>();
        orientation.rotation.x = reader.read<float>();
        orientation.rotation.y = reader.read<float>();
        orientation.rotation.z = reader.read<float>();
    }
}

const vector<ResourcePtr>& ModelGroup::models() const
{
    return _models;
}

const vector<uint32_t>& ModelGroup::parents() const
{
    return _parents;
}

const vector<ModelGroup::Orientation>& ModelGroup::orientations() const
{
    return _orientations;
}
