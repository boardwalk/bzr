#include "ResourceCache.h"
#include "Core.h"
#include "DatFile.h"
#include "SimpleModel.h"

static Destructable* loadResource(uint32_t fileId)
{
    auto data = Core::get().portalDat().read(fileId);

    if(data.empty())
    {
        throw runtime_error("Resource not found");
    }

    switch(fileId >> 24)
    {
        case 0x01:
            return new SimpleModel(data.data(), data.size());
        case 0x02:
            return nullptr;
        default:
            throw runtime_error("File type not supported");
    }
}

shared_ptr<Destructable> ResourceCache::get(uint32_t fileId)
{
    shared_ptr<Destructable> sharedPtr;

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
