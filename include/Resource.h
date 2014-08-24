/*
 * Bael'Zharon's Respite
 * Copyright (C) 2014 Daniel Skorupski
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */
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