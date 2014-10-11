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
#include "Core.h"
#include "DatFile.h"
#include "Environment.h"
#include "ImgColor.h"
#include "ImgTex.h"
#include "Model.h"
#include "MotionTable.h"
#include "Palette.h"
#include "PhysicsScript.h"
#include "PhysicsScriptTable.h"
#include "Region.h"
#include "Scene.h"
#include "Setup.h"
#include "Sound.h"
#include "SoundTable.h"
#include "Surface.h"

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
        case ResourceType::kSetup:
            return new Setup{resourceId, data.data(), data.size()};
        case ResourceType::kAnimation:
            return new Animation{resourceId, data.data(), data.size()};
        case ResourceType::kPalette:
            return new Palette{resourceId, data.data(), data.size()};
        case ResourceType::kImgTex:
            return new ImgTex{resourceId, data.data(), data.size()};
        case ResourceType::kImgColor:
            return new ImgColor{resourceId, data.data(), data.size()};
        case ResourceType::kSurface:
            return new Surface{resourceId, data.data(), data.size()};
        case ResourceType::kMotionTable:
            return new MotionTable{resourceId, data.data(), data.size()};
        case ResourceType::kSound:
            return new Sound{resourceId, data.data(), data.size()};
        case ResourceType::kEnvironment:
            return new Environment{resourceId, data.data(), data.size()};
        case ResourceType::kScene:
            return new Scene{resourceId, data.data(), data.size()};
        case ResourceType::kRegion:
            return new Region{resourceId, data.data(), data.size()};
        case ResourceType::kSoundTable:
            return new SoundTable{resourceId, data.data(), data.size()};
        case ResourceType::kPhysicsScript:
            return new PhysicsScript{resourceId, data.data(), data.size()};
        case ResourceType::kPhysicsScriptTable:
            return new PhysicsScriptTable{resourceId, data.data(), data.size()};
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
