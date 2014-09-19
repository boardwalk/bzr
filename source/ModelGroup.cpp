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
#include "ModelGroup.h"
#include "BinReader.h"
#include "Core.h"
#include "ResourceCache.h"
#include <glm/gtx/norm.hpp>

ModelGroup::ModelGroup(uint32_t id, const void* data, size_t size) : ResourceImpl{id}
{
    BinReader reader(data, size);

    uint32_t resourceId = reader.readInt();
    assert(resourceId == id);

    uint32_t flags = reader.readInt();
    assert(flags <= 0xF);

    uint32_t numModels = reader.readInt();
    modelInfos.resize(numModels);

    for(ModelInfo& modelInfo : modelInfos)
    {
        uint32_t modelId = reader.readInt();
        modelInfo.resource = Core::get().resourceCache().get(modelId);
    }

    if(flags & 1)
    {
        for(ModelInfo& modelInfo : modelInfos)
        {
            modelInfo.parent = reader.readInt();
        }
    }

    if(flags & 2)
    {
        for(ModelInfo& modelInfo : modelInfos)
        {
            modelInfo.scale.x = reader.readFloat();
            modelInfo.scale.y = reader.readFloat();
            modelInfo.scale.z = reader.readFloat();
        }
    }

    uint32_t numExtendedLocs = reader.readInt();

    for(uint32_t i = 0; i < numExtendedLocs; i++)
    {
        reader.readInt();
        reader.readInt();

        reader.readFloat();
        reader.readFloat();
        reader.readFloat();

        glm::quat q;
        q.w = reader.readFloat();
        q.x = reader.readFloat();
        q.y = reader.readFloat();
        q.z = reader.readFloat();

        assert(glm::length2(q) >= 0.99 && glm::length2(q) <= 1.01);
    }

    uint32_t unk1 = reader.readInt();
    assert(unk1 == 0);

    reader.readInt();
    reader.readInt();

    for(ModelInfo& modelInfo : modelInfos)
    {
        modelInfo.position.x = reader.readFloat();
        modelInfo.position.y = reader.readFloat();
        modelInfo.position.z = reader.readFloat();

        modelInfo.rotation.w = reader.readFloat();
        modelInfo.rotation.x = reader.readFloat();
        modelInfo.rotation.y = reader.readFloat();
        modelInfo.rotation.z = reader.readFloat();

        assert(glm::length2(modelInfo.rotation) >= 0.99 && glm::length2(modelInfo.rotation) <= 1.01);
    }
}
