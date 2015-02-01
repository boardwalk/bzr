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
#include "resource/Animation.h"
#include "resource/EnumMapper.h"
#include "resource/Environment.h"
#include "resource/ImgColor.h"
#include "resource/ImgTex.h"
#include "resource/Model.h"
#include "resource/MotionTable.h"
#include "resource/Palette.h"
#include "resource/ParticleEmitter.h"
#include "resource/PhysicsScript.h"
#include "resource/PhysicsScriptTable.h"
#include "resource/Region.h"
#include "resource/Scene.h"
#include "resource/Setup.h"
#include "resource/Sound.h"
#include "resource/SoundTable.h"
#include "resource/Surface.h"
#include "Core.h"
#include "DatFile.h"

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

    ResourceType resourceType = static_cast<ResourceType>(resourceId & 0xFF000000);

    switch(resourceType)
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
        case ResourceType::kEnumMapper:
            return new EnumMapper{resourceId, data.data(), data.size()};
        case ResourceType::kParticleEmitter:
            return new ParticleEmitter{resourceId, data.data(), data.size()};
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
