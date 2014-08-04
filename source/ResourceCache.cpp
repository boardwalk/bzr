#include "ResourceCache.h"
#include "SimpleModel.h"

static Destructable* loadResource(uint32_t fileId)
{
    switch(fileId >> 24)
    {
        case 0x01:
            return new SimpleModel(fileId);
        case 0x02:
            return nullptr;
        default:
            throw runtime_error("File type not supporrted");
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
