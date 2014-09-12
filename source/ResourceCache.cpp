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
#include "ResourceCache.h"
#include "Animation.h"
#include "AnimationSet.h"
#include "Core.h"
#include "DatFile.h"
#include "Palette.h"
#include "Model.h"
#include "ModelGroup.h"
#include "Sound.h"
#include "StructureGeom.h"
#include "Texture.h"
#include "TextureLookup5.h"
#include "TextureLookup8.h"

static Resource* loadResource(uint32_t resourceId)
{
    auto data = Core::get().portalDat().read(resourceId);

    if(data.empty())
    {
        data = Core::get().highresDat().read(resourceId);

        if(data.empty())
        {
            throw runtime_error("Resource not found");
        }
    }

    switch(resourceId & 0xFF000000)
    {
        case ResourceType::Model:
            return new Model(resourceId, data.data(), data.size());
        case ResourceType::ModelGroup:
            return new ModelGroup(resourceId, data.data(), data.size());
        case ResourceType::Animation:
            return new Animation(resourceId, data.data(), data.size());
        case ResourceType::Palette:
            return new Palette(resourceId, data.data(), data.size());
        case ResourceType::TextureLookup5:
            return new TextureLookup5(resourceId, data.data(), data.size());
        case ResourceType::Texture:
            return new Texture(resourceId, data.data(), data.size());
        case ResourceType::TextureLookup8:
            return new TextureLookup8(resourceId, data.data(), data.size());
        case ResourceType::AnimationSet:
            return new AnimationSet(resourceId, data.data(), data.size());
        case ResourceType::Sound:
            return new Sound(resourceId, data.data(), data.size());
        case ResourceType::StructureGeom:
            return new StructureGeom(resourceId, data.data(), data.size());
        default:
            throw runtime_error("Resource type not supported");
    }
}

ResourcePtr ResourceCache::get(uint32_t resourceId)
{
    ResourcePtr sharedPtr;

    auto& weakPtr = _data[resourceId];

    if(weakPtr.expired())
    {
        sharedPtr.reset(loadResource(resourceId));
        weakPtr = sharedPtr;
    }
    else
    {
        sharedPtr = weakPtr.lock();
    }

    return sharedPtr;
}
