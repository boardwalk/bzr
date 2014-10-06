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
#include "Region.h"
#include "Scene.h"
#include "Sound.h"
#include "Surface.h"
#include "StructureGeom.h"
#include "Texture.h"
#include "TextureLookup5.h"

static const Resource* loadResource(uint32_t resourceId)
{
    vector<uint8_t> data = Core::get().portalDat().read(resourceId);

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
        case ResourceType::kModel:
            return new Model{resourceId, data.data(), data.size()};
        case ResourceType::kModelGroup:
            return new ModelGroup{resourceId, data.data(), data.size()};
        case ResourceType::kAnimation:
            return new Animation{resourceId, data.data(), data.size()};
        case ResourceType::kPalette:
            return new Palette{resourceId, data.data(), data.size()};
        case ResourceType::kTextureLookup5:
            return new TextureLookup5{resourceId, data.data(), data.size()};
        case ResourceType::kTexture:
            return new Texture{resourceId, data.data(), data.size()};
        case ResourceType::kSurface:
            return new Surface{resourceId, data.data(), data.size()};
        case ResourceType::kAnimationSet:
            return new AnimationSet{resourceId, data.data(), data.size()};
        case ResourceType::kSound:
            return new Sound{resourceId, data.data(), data.size()};
        case ResourceType::kStructureGeom:
            return new StructureGeom{resourceId, data.data(), data.size()};
        case ResourceType::kScene:
            return new Scene{resourceId, data.data(), data.size()};
        case ResourceType::kRegion:
            return new Region{resourceId, data.data(), data.size()};
        default:
            throw runtime_error("Resource type not supported");
    }
}

ResourcePtr ResourceCache::get(uint32_t resourceId)
{
    weak_ptr<const Resource>& weakPtr = data_[resourceId];

    ResourcePtr sharedPtr = weakPtr.lock();

    if(!sharedPtr)
    {
        sharedPtr.reset(loadResource(resourceId));
        weakPtr = sharedPtr;
    }

    return sharedPtr;
}
