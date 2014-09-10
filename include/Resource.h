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
        Model = 0x01000000,
        ModelGroup = 0x02000000,
        Animation = 0x03000000,
        Palette = 0x04000000,
        TextureLookup5 = 0x05000000,
        Texture = 0x06000000,
        TextureLookup8 = 0x08000000,
        StructureGeom = 0x0D000000
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

    uint32_t resourceId() const
    {
        return _resourceId;
    }

    ResourceType::Value resourceType() const
    {
        return static_cast<ResourceType::Value>(_resourceId & 0xFF000000);
    }

private:
    const uint32_t _resourceId;
};

template<ResourceType::Value RT>
class ResourceImpl : public Resource
{
public:
    ResourceImpl(uint32_t id) : Resource(id)
    {
        assert((id & 0xFF000000) == RT);
    }

    static const ResourceType::Value RESOURCE_TYPE = RT;
};

typedef shared_ptr<Resource> ResourcePtr;

#endif