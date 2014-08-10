#include "ModelGroup.h"
#include "BlobReader.h"
#include "Core.h"
#include "ResourceCache.h"

ModelGroup::ModelGroup(uint32_t id, const void* data, size_t size) : ResourceImpl(id)
{
    BlobReader reader(data, size);

    auto resourceId = reader.read<uint32_t>();
    assert(resourceId == id);
    assert((resourceId & 0xFF000000) == 0x02000000);

    auto flags = reader.read<uint32_t>();
    assert(flags <= 0xF);

    auto modelCount = reader.read<uint32_t>();
    _modelInfos.resize(modelCount);

    for(auto& modelInfo : _modelInfos)
    {
        auto modelId = reader.read<uint32_t>();
        modelInfo.resource = Core::get().resourceCache().get(modelId);
    }

    if(flags & 1)
    {
        for(auto& modelInfo : _modelInfos)
        {
            modelInfo.parent = reader.read<uint32_t>();
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

    for(auto& modelInfo : _modelInfos)
    {
        modelInfo.position.x = reader.read<float>();
        modelInfo.position.y = reader.read<float>();
        modelInfo.position.z = reader.read<float>();

        modelInfo.rotation.w = reader.read<float>();
        modelInfo.rotation.x = reader.read<float>();
        modelInfo.rotation.y = reader.read<float>();
        modelInfo.rotation.z = reader.read<float>();
    }
}

const vector<ModelGroup::ModelInfo>& ModelGroup::modelInfos() const
{
    return _modelInfos;
}
