#ifndef BZR_RESOURCE_H
#define BZR_RESOURCE_H

#include "Noncopyable.h"

class Resource : Noncopyable
{
public:
    enum ResourceType
    {
        Model,
        ModelGroup,
        Palette,
        TextureLookup5,
        Texture,
        TextureLookup8
    };

    Resource(uint32_t id) : _resourceId(id)
    {}

    virtual ~Resource()
    {}

    template<class T>
    T& cast()
    {
        assert(resourceType() == T::RESOURCE_TYPE);
        return (T&)*this;
    }

    template<class T>
    const T& cast() const
    {
        assert(resourceType() == T::RESOURCE_TYPE);
        return (const T&)*this;
    }

    virtual ResourceType resourceType() const = 0;

    uint32_t resourceId() const
    {
        return _resourceId;
    }

private:
    const uint32_t _resourceId;
};

template<Resource::ResourceType RT>
class ResourceImpl : public Resource
{
public:
    ResourceImpl(uint32_t id) : Resource(id)
    {}

    Resource::ResourceType resourceType() const override
    {
        return RT;
    }

    static const Resource::ResourceType RESOURCE_TYPE = RT;
};

typedef shared_ptr<Resource> ResourcePtr;

#endif