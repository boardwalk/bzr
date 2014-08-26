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
#include "BlobReader.h"
#include "Core.h"
#include "ResourceCache.h"

//#define DEBUG_MODEL (resourceId == 0x02000120)
#define DEBUG_MODEL ((resourceId & 0xFF000000) == 0x02000000)

ModelGroup::ModelGroup(uint32_t id, const void* data, size_t size) : ResourceImpl(id)
{
    BlobReader reader(data, size);

    auto resourceId = reader.read<uint32_t>();
    assert(resourceId == id);
    assert((resourceId & 0xFF000000) == 0x02000000);

    auto flags = reader.read<uint32_t>();
    assert(flags <= 0xF);

    auto modelCount = reader.read<uint32_t>();
    _modelInfos.resize(modelCount);

    for(auto& modelInfo : _modelInfos)
    {
        auto modelId = reader.read<uint32_t>();
        modelInfo.resource = Core::get().resourceCache().get(modelId);
    }

    if(flags & 1)
    {
        for(auto& modelInfo : _modelInfos)
        {
            modelInfo.parent = reader.read<uint32_t>();
        }
    }

    if(flags & 2)
    {
        for(auto& modelInfo : _modelInfos)
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

        Quat q;
        q.w = reader.read<float>();
        q.x = reader.read<float>();
        q.y = reader.read<float>();
        q.z = reader.read<float>();
        
        assert(q.norm() >= 0.999 && q.norm() <= 1.001);
    }

    auto b = reader.read<uint32_t>();
    auto c = reader.read<uint32_t>();
    auto d = reader.read<uint32_t>();

    for(auto& modelInfo : _modelInfos)
    {
        modelInfo.position.x = reader.read<float>();
        modelInfo.position.y = reader.read<float>();
        modelInfo.position.z = reader.read<float>();

        modelInfo.rotation.w = reader.read<float>();
        modelInfo.rotation.x = reader.read<float>();
        modelInfo.rotation.y = reader.read<float>();
        modelInfo.rotation.z = reader.read<float>();

        assert(modelInfo.rotation.norm() >= 0.999 && modelInfo.rotation.norm() <= 1.001);
    }

    if(DEBUG_MODEL)
    {
        printf("ModelGroup::ModelGroup(%08x): flags=%08x\n", resourceId, flags);
        printf("ModelGroup::ModelGroup(%08x): unk=%08x %08x %08x\n", resourceId, b, c, d);

        int i = 0;

        for(auto& modelInfo : _modelInfos)
        {
            printf("ModelGroup::ModelGroup(%08x): %02x model=%08x\n", resourceId, i, modelInfo.resource->resourceId());
            printf("ModelGroup::ModelGroup(%08x): %02x parent=%08x\n", resourceId, i, modelInfo.parent);
            printf("ModelGroup::ModelGroup(%08x): %02x scale=%.2f %.2f %.2f\n", resourceId, i, modelInfo.scale.x, modelInfo.scale.y, modelInfo.scale.z);
            printf("ModelGroup::ModelGroup(%08x): %02x pos=%.2f %.2f %.2f rot=%.2f %.2f %.2f %.2f\n\n", resourceId, i,
                modelInfo.position.x, modelInfo.position.y, modelInfo.position.z,
                modelInfo.rotation.w, modelInfo.rotation.x, modelInfo.rotation.y, modelInfo.rotation.z);

            i++;
        }

    }
}

const vector<ModelGroup::ModelInfo>& ModelGroup::modelInfos() const
{
    return _modelInfos;
}
