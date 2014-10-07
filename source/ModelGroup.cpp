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
#include "physics/CylSphere.h"
#include "physics/Sphere.h"
#include "BinReader.h"
#include "Core.h"
#include "ResourceCache.h"

enum ModelGroupFlag
{
    kHasParentIndex = 0x1,
    kHasDefaultScale = 0x2,
    kAllowFreeHeading = 0x4,
    kHasPhysicsBSP = 0x8
};

ModelGroup::ModelGroup(uint32_t id, const void* data, size_t size) : ResourceImpl{id}
{
    BinReader reader(data, size);

    uint32_t resourceId = reader.readInt();
    assert(resourceId == id);

    uint32_t flags = reader.readInt();
    assert(flags <= 0xF);

    uint32_t numModels = reader.readInt();

    models.reserve(numModels);

    for(uint32_t i = 0; i < numModels; i++)
    {
        uint32_t modelId = reader.readInt();
        models.emplace_back(Core::get().resourceCache().get(modelId));
    }

    parents.reserve(numModels);

    for(uint32_t i = 0; i < numModels; i++)
    {
        uint32_t parent = 0xFFFFFFFF;

        if(flags & kHasParentIndex)
        {
            parent = reader.readInt();
        }

        parents.push_back(parent);
    }

    scales.reserve(numModels);

    for(uint32_t i = 0; i < numModels; i++)
    {
        glm::vec3 scale{1.0, 1.0, 1.0};

        if(flags & kHasDefaultScale)
        {
            scale.x = reader.readFloat();
            scale.y = reader.readFloat();
            scale.z = reader.readFloat();
        }

        scales.push_back(scale);
    }

    uint32_t numHoldingLocations = reader.readInt();

    for(uint32_t i = 0; i < numHoldingLocations; i++)
    {
        /*key*/ reader.readInt();
        /*partIndex*/ reader.readInt();

        Location().read(reader);
    }

    uint32_t numConnectionPoints = reader.readInt();
    assert(numConnectionPoints == 0);

    for(uint32_t i = 0; i < numConnectionPoints; i++)
    {
        /*key*/ reader.readInt();
        /*partIndex*/ reader.readInt();

        Location().read(reader);
    }

    uint32_t numPlacementFrames = reader.readInt();
    placementFrames.reserve(numPlacementFrames);

    for(uint32_t i = 0; i < numPlacementFrames; i++)
    {
        /*key*/ reader.readInt();

        placementFrames.emplace_back(reader, numModels);
    }

    uint32_t numCylSpheres = reader.readInt();

    for(uint32_t i = 0; i < numCylSpheres; i++)
    {
        CylSphere().read(reader);
    }

    uint32_t numSpheres = reader.readInt();

    for(uint32_t i = 0; i < numSpheres; i++)
    {
        Sphere().read(reader);
    }

    /*height*/ reader.readFloat();
    /*radius*/ reader.readFloat();
    /*stepUpHeight*/ reader.readFloat();
    /*stepDownHeight*/ reader.readFloat();

    // sorting sphere
    Sphere().read(reader);

    // selection sphere
    Sphere().read(reader);

    uint32_t numLights = reader.readInt();

    for(uint32_t i = 0; i < numLights; i++)
    {
        uint32_t lightIndex = reader.readInt();
        assert(lightIndex == i);

        Location().read(reader);

        /*color*/ reader.readInt();
        /*intensity*/ reader.readFloat();
        /*falloff*/ reader.readFloat();
        /*coneAngle*/ reader.readFloat(); // junk 0xcdcdcdcd most of the time
    }

    /*defaultAnimId*/ reader.readInt();
    /*defaultScriptId*/ reader.readInt();
    /*defaultMTableId*/ reader.readInt();
    /*defaultStableId*/ reader.readInt();
    /*defaultPhstableId*/ reader.readInt();

    reader.assertEnd();
}
