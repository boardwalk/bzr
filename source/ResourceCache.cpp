#include "ResourceCache.h"
#include "Core.h"
#include "DatFile.h"
#include "Palette.h"
#include "Model.h"
#include "TextureLookup.h"

static Resource* loadResource(uint32_t fileId)
{
    auto data = Core::get().portalDat().read(fileId);

    if(data.empty())
    {
        throw runtime_error("Resource not found");
    }

    switch(fileId >> 24)
    {
        case 0x01:
            return new Model(data.data(), data.size());
        case 0x02:
            return nullptr;
        case 0x04:
            return new Palette(data.data(), data.size());
        case 0x05:
            return new TextureLookup(data.data(), data.size());
        default:
            throw runtime_error("File type not supported");
    }
}

ResourcePtr ResourceCache::get(uint32_t fileId)
{
    ResourcePtr sharedPtr;

    auto& weakPtr = _data[fileId];

    if(weakPtr.expired())
    {
        sharedPtr.reset(loadResource(fileId));
        weakPtr = sharedPtr;
    }
    else
    {
        sharedPtr = weakPtr.lock();
    }

    return sharedPtr;
}
