#ifndef BZR_RESOURCE_H
#define BZR_RESOURCE_H

class Resource
{
public:
    enum ResourceType
    {
        Model,
        Palette,
        TextureLookup
    };

    virtual ~Resource() {}

    template<class T>
    T& cast()
    {
        assert(type() == T::TYPE);
        return (T&)*this;
    }

    template<class T>
    const T& cast() const
    {
        assert(type() == T::TYPE);
        return (const T&)*this;
    }

    virtual ResourceType type() const = 0;
};

template<Resource::ResourceType RT>
class ResourceImpl : public Resource
{
public:
    static const Resource::ResourceType TYPE = RT;

    Resource::ResourceType type() const override
    {
        return RT;
    }
};

typedef shared_ptr<Resource> ResourcePtr;

#endif