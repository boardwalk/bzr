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
#include "Scene.h"
#include "BinReader.h"
#include "Land.h"

static void read(BinReader& reader, Scene::ObjectDesc& objectDesc)
{
    objectDesc.resourceId = reader.readInt();
    assert(objectDesc.resourceId == 0 || (objectDesc.resourceId & 0xFF000000) == ResourceType::kModel || (objectDesc.resourceId & 0xFF000000) == ResourceType::kSetup);

    objectDesc.position.x = reader.readFloat();
    assert(objectDesc.position.x >= -Land::kCellSize && objectDesc.position.x <= Land::kCellSize);

    objectDesc.position.y = reader.readFloat();
    assert(objectDesc.position.y >= -Land::kCellSize && objectDesc.position.y <= Land::kCellSize);

    objectDesc.position.z = reader.readFloat();

    objectDesc.rotation.w = reader.readFloat();
    objectDesc.rotation.x = reader.readFloat();
    objectDesc.rotation.y = reader.readFloat();
    objectDesc.rotation.z = reader.readFloat();

    objectDesc.frequency = reader.readFloat();
    assert(objectDesc.frequency >= 0.0 && objectDesc.frequency <= 1.0);

    objectDesc.displace.x = reader.readFloat();
    assert(objectDesc.displace.x >= -Land::kCellSize && objectDesc.displace.x <= Land::kCellSize);

    objectDesc.displace.y = reader.readFloat();
    assert(objectDesc.displace.y >= -Land::kCellSize && objectDesc.displace.y <= Land::kCellSize);

    objectDesc.minScale = reader.readFloat();
    assert(objectDesc.minScale >= 0.0f);

    objectDesc.maxScale = reader.readFloat();
    assert(objectDesc.maxScale >= 0.0f);
    assert(objectDesc.minScale <= objectDesc.maxScale);

    objectDesc.maxRotation = reader.readFloat();
    assert(objectDesc.maxRotation >= 0.0 && objectDesc.maxRotation <= 360.0);

    objectDesc.minSlope = reader.readFloat();
    objectDesc.maxSlope = reader.readFloat();

    uint32_t intAlign = reader.readInt();
    assert(intAlign == 0 || intAlign == 1);
    objectDesc.align = (intAlign != 0);

    uint32_t intOrient = reader.readInt();
    assert(intOrient == 0 || intOrient == 1);
    objectDesc.orient = (intOrient != 0);

    uint32_t intIsWeenieObj = reader.readInt();
    assert(intIsWeenieObj == 0 || intIsWeenieObj == 1);
    objectDesc.isWeenieObj = (intIsWeenieObj != 0);
}

Scene::Scene(uint32_t id, const void* data, size_t size) : ResourceImpl{id}
{
    BinReader reader(data, size);

    uint32_t resourceId = reader.readInt();
    assert(resourceId == id);

    uint32_t numObjects = reader.readInt();
    objects.resize(numObjects);

    for(ObjectDesc& objectDesc : objects)
    {
        read(reader, objectDesc);
    }
}
