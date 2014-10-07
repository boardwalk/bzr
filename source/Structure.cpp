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
#include "Structure.h"
#include "BinReader.h"
#include "Core.h"
#include "Environment.h"
#include "ResourceCache.h"

enum EnvCellFlags
{
    kSeenOutside = 1,
    kHasStaticObjects = 2,
    kHasWeenieObjects = 4,
    kHasRestrictionObject = 8
};

Structure::Structure(const void* data, size_t size)
{
    BinReader reader(data, size);

    uint32_t resourceId = reader.readInt();
    id_ = LandcellId(resourceId);

    uint32_t flags = reader.readInt();
    assert(flags <= 0xF);

    uint32_t resourceId2 = reader.readInt();
    assert(resourceId2 == resourceId);

    uint8_t numSurfaces = reader.readByte();
    surfaces_.resize(numSurfaces);

    uint8_t numConnected = reader.readByte();
    uint16_t numVisible = reader.readShort();

    for(ResourcePtr& surface : surfaces_)
    {
        uint16_t surfaceId = reader.readShort();
        surface = Core::get().resourceCache().get(ResourceType::kSurface | surfaceId);
    }

    uint16_t environmentId = reader.readShort();
    environment_ = Core::get().resourceCache().get(ResourceType::kEnvironment | environmentId);
    partNum_ = reader.readShort();
    location_.read(reader);

    // struct CCellPortal
    for(uint8_t i = 0; i < numConnected; i++)
    {
        /*portalSide*/ reader.readShort();
        /*portalId*/ reader.readShort();
        /*cellId*/ reader.readShort();
        /*exactMatch*/ reader.readShort();
    }

    for(uint16_t i = 0; i < numVisible; i++)
    {
        /*cellId*/ reader.readShort();
    }

    if(flags & kHasStaticObjects)
    {
        uint32_t numStaticObjects = reader.readInt();
        staticObjects_.reserve(numStaticObjects);

        for(uint32_t i = 0; i < numStaticObjects; i++)
        {
            staticObjects_.emplace_back(reader);
        }
    }

    if(flags & kHasRestrictionObject)
    {
        reader.readInt();
    }

    reader.assertEnd();
}

LandcellId Structure::id() const
{
    return id_;
}

const Location& Structure::location() const
{
    return location_;
}

const vector<ResourcePtr>& Structure::surfaces() const
{
    return surfaces_;
}

const Environment& Structure::environment() const
{
    return environment_->cast<Environment>();
}

uint16_t Structure::partNum() const
{
    return partNum_;
}
