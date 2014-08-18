#ifndef BZR_RESOURCE_H
#define BZR_RESOURCE_H

#include "Noncopyable.h"

struct ResourceType
{
    enum Value
    {
        Model,
        ModelGroup,
        Palette,
        TextureLookup5,
        Texture,
        TextureLookup8,
        StructureGeom
    };
};

class Resource : Noncopyable
{
public:
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

    virtual ResourceType::Value resourceType() const = 0;

    uint32_t resourceId() const
    {
        return _resourceId;
    }

private:
    const uint32_t _resourceId;
};

template<ResourceType::Value RT>
class ResourceImpl : public Resource
{
public:
    ResourceImpl(uint32_t id) : Resource(id)
    {}

    ResourceType::Value resourceType() const override
    {
        return RT;
    }

    static const ResourceType::Value RESOURCE_TYPE = RT;
};

typedef shared_ptr<Resource> ResourcePtr;

#endif