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
        kModel = 0x01000000,
        kModelGroup = 0x02000000,
        kAnimation = 0x03000000,
        kPalette = 0x04000000,
        kTextureLookup5 = 0x05000000,
        kTexture = 0x06000000,
        kTextureLookup8 = 0x08000000,
        kAnimationSet = 0x09000000,
        kSound = 0x0A000000,
        kStructureGeom = 0x0D000000
    };
};

class Resource : Noncopyable
{
public:
    Resource(uint32_t id) : resourceId_(id)
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
        return resourceId_;
    }

    ResourceType::Value resourceType() const
    {
        return static_cast<ResourceType::Value>(resourceId_ & 0xFF000000);
    }

private:
    const uint32_t resourceId_;
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