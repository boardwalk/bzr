#ifndef BZR_RESOURCE_H
#define BZR_RESOURCE_H

class Resource
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

    virtual ~Resource() {}

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
};

template<Resource::ResourceType RT>
class ResourceImpl : public Resource
{
public:
    static const Resource::ResourceType RESOURCE_TYPE = RT;

    Resource::ResourceType resourceType() const override
    {
        return RT;
    }
};

typedef shared_ptr<Resource> ResourcePtr;

#endif