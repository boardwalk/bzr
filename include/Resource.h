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

/*
 * 00 GID_TYPE_WEENIE_DEFS (0)
 * 01 GID_TYPE_GFXOBJ (15105)
 * 02 GID_TYPE_SETUP (5852)
 * 03 GID_TYPE_ANIM (2055)
 * 04 GID_TYPE_PALETTE (4515)
 * 05 GID_TYPE_IMGTEX (7152)
 * 06 GID_TYPE_IMGCOLOR (20547)
 * 08 GID_TYPE_SURFACE (6080)
 * 09 GID_TYPE_MTABLE (427)
 * 0a GID_TYPE_WAVE (783)
 * 0b GID_TYPE_LAND_BLOCK (0)
 * 0c GID_TYPE_LBI (0)
 * 0d GID_TYPE_ENVIRONMENT (769)
 * 0e GID_TYPE_UNIQUE (14)
 * 0f GID_TYPE_PAL_SET (2676)
 * 10 GID_TYPE_CLOTHING (1909)
 * 11 GID_TYPE_DEGRADEINFO (4126)
 * 12 GID_TYPE_SCENE (179)
 * 13 GID_TYPE_REGION (1)
 * 14 ?? (2)
 * 15 ?? (2)
 * 16 ?? (1)
 * 17 ?? (1)
 * 18 ?? (1)
 * 20 GID_TYPE_STABLE (189)
 * 22 GID_TYPE_ENUM_MAPPER (40)
 * 25 GID_TYPE_DID_MAPPER (22)
 * 26 ?? (1)
 * 27 ?? (5)
 * 30 GID_TYPE_COMBAT_TABLE (71)
 * 31 GID_TYPE_STRING (28)
 * 32 GID_TYPE_PARTICLE_EMITTER (2006)
 * 33 GID_TYPE_PHYSICS_SCRIPT (4189)
 * 34 GID_TYPE_PHYSICS_SCRIPT_TABLE (160)
 * 38 GID_TYPE_MUTATE_FILTER (0)
 * 39 ?? (1)
 * 40 ?? (49)
 * 78 ?? (2)
 * ff ?? (1)
 */

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
        kSurface = 0x08000000,
        kMotionTable = 0x09000000,
        kSound = 0x0A000000,
        kEnvironment = 0x0D000000,
        kScene = 0x12000000,
        kRegion = 0x13000000
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

typedef shared_ptr<const Resource> ResourcePtr;

#endif