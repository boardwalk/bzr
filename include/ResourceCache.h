#ifndef BZR_RESOURCECACHE_H
#define BZR_RESOURCECACHE_H

#include "Destructable.h"
#include "Noncopyable.h"
#include <unordered_map>

class ResourceCache : Noncopyable
{
public:
    shared_ptr<Destructable> get(uint32_t fileId);

private:
    unordered_map<uint32_t, weak_ptr<Destructable>> _data;
};

#endif