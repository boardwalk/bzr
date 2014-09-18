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

    auto resourceId = reader.read<uint32_t>();
    id_ = LandcellId(resourceId);

    // 0x1 above ground
    // 0x2 has objects
    // 0x4 unknown
    // 0x8 unknown, extra 4 bytes
    auto flags = reader.read<uint32_t>();
    assert(flags <= 0xF);

    auto resourceId2 = reader.read<uint32_t>();
    assert(resourceId2 == resourceId);

    auto numTextures = reader.read<uint8_t>();
    textures_.resize(numTextures);

    auto numConnected = reader.read<uint8_t>();
    auto numVisible = reader.read<uint16_t>();

    for(auto& texture : textures_)
    {
        auto textureId = reader.read<uint16_t>();
        texture = Core::get().resourceCache().get(ResourceType::TextureLookup8 | textureId);
    }

    auto geometryId = reader.read<uint16_t>();
    geometry_ = Core::get().resourceCache().get(ResourceType::StructureGeom | geometryId);

    partNum_ = reader.read<uint16_t>();

    position_.x = reader.read<float>();
    position_.y = reader.read<float>();
    position_.z = reader.read<float>();

    rotation_.w = reader.read<float>();
    rotation_.x = reader.read<float>();
    rotation_.y = reader.read<float>();
    rotation_.z = reader.read<float>();

    for(auto ci = 0u; ci < numConnected; ci++)
    {
        reader.read<uint16_t>();
        reader.read<uint16_t>();
        reader.read<uint16_t>(); // structure index
        reader.read<uint16_t>();
    }

    for(auto vi = 0u; vi < numVisible; vi++)
    {
        reader.read<uint16_t>(); // structure index
    }

    if(flags & 2)
    {
        auto numDoodads = reader.read<uint32_t>();
        doodads_.resize(numDoodads);

        for(auto& doodad : doodads_)
        {
            doodad.read(reader);
        }
    }

    if(flags & 8)
    {
        // I'm not sure this is where this is supposed to be
        reader.read<uint32_t>();
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
