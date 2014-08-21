#include "Object.h"
#include "BlobReader.h"
#include "Core.h"
#include "ResourceCache.h"

void Object::read(BlobReader& reader)
{
    auto modelId = reader.read<uint32_t>();
    resource = Core::get().resourceCache().get(modelId);

    position.x = reader.read<float>();
    position.y = reader.read<float>();
    position.z = reader.read<float>();

    rotation.w = reader.read<float>();
    rotation.x = reader.read<float>();
    rotation.y = reader.read<float>();
    rotation.z = reader.read<float>();
}
