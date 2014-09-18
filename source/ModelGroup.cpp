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

ModelGroup::ModelGroup(uint32_t id, const void* data, size_t size) : ResourceImpl(id)
{
    BinReader reader(data, size);

    auto resourceId = reader.read<uint32_t>();
    assert(resourceId == id);

    auto flags = reader.read<uint32_t>();
    assert(flags <= 0xF);

    auto numModels = reader.read<uint32_t>();
    modelInfos_.resize(numModels);

    for(auto& modelInfo : modelInfos_)
    {
        auto modelId = reader.read<uint32_t>();
        modelInfo.resource = Core::get().resourceCache().get(modelId);
    }

    if(flags & 1)
    {
        for(auto& modelInfo : modelInfos_)
        {
            modelInfo.parent = reader.read<uint32_t>();
        }
    }

    if(flags & 2)
    {
        for(auto& modelInfo : modelInfos_)
        {
            modelInfo.scale.x = reader.read<float>();
            modelInfo.scale.y = reader.read<float>();
            modelInfo.scale.z = reader.read<float>();
        }
    }

    auto numExtendedLocs = reader.read<uint32_t>();

    for(auto i = 0u; i < numExtendedLocs; i++)
    {
        reader.read<uint32_t>();
        reader.read<uint32_t>();

        reader.read<float>();
        reader.read<float>();
        reader.read<float>();

        glm::quat q;
        q.w = reader.read<float>();
        q.x = reader.read<float>();
        q.y = reader.read<float>();
        q.z = reader.read<float>();

        assert(glm::length2(q) >= 0.99 && glm::length2(q) <= 1.01);
    }

    auto unk1 = reader.read<uint32_t>();
    assert(unk1 == 0);

    reader.read<uint32_t>();
    reader.read<uint32_t>();

    for(auto& modelInfo : modelInfos_)
    {
        modelInfo.position.x = reader.read<float>();
        modelInfo.position.y = reader.read<float>();
        modelInfo.position.z = reader.read<float>();

        modelInfo.rotation.w = reader.read<float>();
        modelInfo.rotation.x = reader.read<float>();
        modelInfo.rotation.y = reader.read<float>();
        modelInfo.rotation.z = reader.read<float>();

        assert(glm::length2(modelInfo.rotation) >= 0.99 && glm::length2(modelInfo.rotation) <= 1.01);
    }
}

const vector<ModelGroup::ModelInfo>& ModelGroup::modelInfos() const
{
    return modelInfos_;
}
