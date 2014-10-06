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

void Scene::ObjectDesc::read(BinReader& reader)
{
    resourceId = reader.readInt();
    assert(resourceId == 0 || (resourceId & 0xFF000000) == ResourceType::kModel || (resourceId & 0xFF000000) == ResourceType::kModelGroup);

    position.x = reader.readFloat();
    assert(position.x >= -Land::kCellSize && position.x <= Land::kCellSize);

    position.y = reader.readFloat();
    assert(position.y >= -Land::kCellSize && position.y <= Land::kCellSize);

    position.z = reader.readFloat();

    rotation.w = reader.readFloat();
    rotation.x = reader.readFloat();
    rotation.y = reader.readFloat();
    rotation.z = reader.readFloat();

    frequency = reader.readFloat();
    assert(frequency >= 0.0 && frequency <= 1.0);

    displace.x = reader.readFloat();
    assert(displace.x >= -Land::kCellSize && displace.x <= Land::kCellSize);

    displace.y = reader.readFloat();
    assert(displace.y >= -Land::kCellSize && displace.y <= Land::kCellSize);

    minScale = reader.readFloat();
    assert(minScale >= 0.0f);

    maxScale = reader.readFloat();
    assert(maxScale >= 0.0f);
    assert(minScale <= maxScale);

    maxRotation = reader.readFloat();
    assert(maxRotation >= 0.0 && maxRotation <= 360.0);

    minSlope = reader.readFloat();
    maxSlope = reader.readFloat();

    uint32_t intAlign = reader.readInt();
    assert(intAlign == 0 || intAlign == 1);
    align = (intAlign != 0);

    uint32_t intOrient = reader.readInt();
    assert(intOrient == 0 || intOrient == 1);
    orient = (intOrient != 0);

    uint32_t intIsWeenieObj = reader.readInt();
    assert(intIsWeenieObj == 0 || intIsWeenieObj == 1);
    isWeenieObj = (intIsWeenieObj != 0);
}

Scene::Scene(uint32_t id, const void* data, size_t size) : ResourceImpl{id}
{
    BinReader reader(data, size);

    uint32_t resourceId = reader.readInt();
    assert(resourceId == id);

    uint32_t numObjects = reader.readInt();
    objects.resize(numObjects);

    for(ObjectDesc& object : objects)
    {
        object.read(reader);
    }
}
