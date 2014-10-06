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
#include "ResourceCache.h"
#include "StructureGeom.h"

Structure::Structure(const void* data, size_t size)
{
    BinReader reader(data, size);

    uint32_t resourceId = reader.readInt();
    id_ = LandcellId(resourceId);

    // 0x1 above ground
    // 0x2 has objects
    // 0x4 unknown
    // 0x8 unknown, extra 4 bytes
    uint32_t flags = reader.readInt();
    assert(flags <= 0xF);

    uint32_t resourceId2 = reader.readInt();
    assert(resourceId2 == resourceId);

    uint8_t numTextures = reader.readByte();
    textures_.resize(numTextures);

    uint8_t numConnected = reader.readByte();
    uint16_t numVisible = reader.readShort();

    for(ResourcePtr& texture : textures_)
    {
        uint16_t textureId = reader.readShort();
        texture = Core::get().resourceCache().get(ResourceType::kSurface | textureId);
    }

    uint16_t geometryId = reader.readShort();
    geometry_ = Core::get().resourceCache().get(ResourceType::kStructureGeom | geometryId);

    partNum_ = reader.readShort();

    position_.x = reader.readFloat();
    position_.y = reader.readFloat();
    position_.z = reader.readFloat();

    rotation_.w = reader.readFloat();
    rotation_.x = reader.readFloat();
    rotation_.y = reader.readFloat();
    rotation_.z = reader.readFloat();

    for(uint8_t ci = 0; ci < numConnected; ci++)
    {
        reader.readShort();
        reader.readShort();
        reader.readShort(); // structure index
        reader.readShort();
    }

    for(uint16_t vi = 0; vi < numVisible; vi++)
    {
        reader.readShort(); // structure index
    }

    if(flags & 2)
    {
        uint32_t numDoodads = reader.readInt();
        doodads_.reserve(numDoodads);

        for(uint32_t di = 0; di < numDoodads; di++)
        {
            doodads_.emplace_back(reader);
        }
    }

    if(flags & 8)
    {
        // I'm not sure this is where this is supposed to be
        reader.readInt();
    }

    reader.assertEnd();
}

LandcellId Structure::id() const
{
    return id_;
}

const glm::vec3& Structure::position() const
{
    return position_;
}

const glm::quat& Structure::rotation() const
{
    return rotation_;
}

const vector<ResourcePtr>& Structure::textures() const
{
    return textures_;
}

const StructureGeom& Structure::geometry() const
{
    return geometry_->cast<StructureGeom>();
}

uint16_t Structure::partNum() const
{
    return partNum_;
}
