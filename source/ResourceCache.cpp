#include "ResourceCache.h"
#include "Core.h"
#include "DatFile.h"
#include "Palette.h"
#include "Model.h"
#include "ModelGroup.h"
#include "StructureGeom.h"
#include "Texture.h"
#include "TextureLookup5.h"
#include "TextureLookup8.h"

static Resource* loadResource(uint32_t resourceId)
{
    auto data = Core::get().portalDat().read(resourceId);

    if(data.empty())
    {
        data = Core::get().highresDat().read(resourceId);

        if(data.empty())
        {
            throw runtime_error("Resource not found");
        }
    }

    switch(resourceId >> 24)
    {
        case 0x01:
            return new Model(resourceId, data.data(), data.size());
        case 0x02:
            return new ModelGroup(resourceId, data.data(), data.size());
        case 0x04:
            return new Palette(resourceId, data.data(), data.size());
        case 0x05:
            return new TextureLookup5(resourceId, data.data(), data.size());
        case 0x06:
            return new Texture(resourceId, data.data(), data.size());
        case 0x08:
            return new TextureLookup8(resourceId, data.data(), data.size());
        case 0x0D:
            return new StructureGeom(resourceId, data.data(), data.size());
        default:
            throw runtime_error("Resource type not supported");
    }
}

ResourcePtr ResourceCache::get(uint32_t resourceId)
{
    ResourcePtr sharedPtr;

    auto& weakPtr = _data[resourceId];

    if(weakPtr.expired())
    {
        sharedPtr.reset(loadResource(resourceId));
        weakPtr = sharedPtr;
    }
    else
    {
        sharedPtr = weakPtr.lock();
    }

    return sharedPtr;
}
