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
#ifndef BZR_MODELGROUP_H
#define BZR_MODELGROUP_H

#include "Resource.h"

struct ModelGroup : public ResourceImpl<ResourceType::kModelGroup>
{
    struct ModelInfo
    {
        ModelInfo() : parent(0xFFFFFFFF), scale(1.0, 1.0, 1.0)
        {}

        ResourcePtr resource;
        uint32_t parent;
        glm::vec3 position;
        glm::quat rotation;
        glm::vec3 scale;
    };

    ModelGroup(uint32_t id, const void* data, size_t size);

    vector<ModelInfo> modelInfos;
};

#endif