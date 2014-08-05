#ifndef BZR_RESOURCECACHE_H
#define BZR_RESOURCECACHE_H

#include "Noncopyable.h"
#include "Resource.h"
#include <unordered_map>

class ResourceCache : Noncopyable
{
public:
    ResourcePtr get(uint32_t fileId);

private:
    unordered_map<uint32_t, weak_ptr<Resource>> _data;
};

#endif