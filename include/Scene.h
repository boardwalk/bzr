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
#ifndef BZR_SCENE_H
#define BZR_SCENE_H

#include "Resource.h"

// AC: Scene
struct Scene : public ResourceImpl<ResourceType::kScene>
{
    // AC: ObjectDesc
    struct ObjectDesc
    {
        uint32_t resourceId;
        glm::vec3 position;
        glm::quat rotation;
        fp_t frequency;
        glm::vec2 displace;
        fp_t minScale;
        fp_t maxScale;
        fp_t maxRotation;
        fp_t minSlope;
        fp_t maxSlope;
        bool align;
        bool orient;
        bool isWeenieObj;
    };

    Scene(uint32_t id, const void* data, size_t size);

    vector<ObjectDesc> objects;
};

#endif