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
#include "Region.h"
#include "BinReader.h"
#include "Core.h"
#include "ResourceCache.h"

static const uint32_t kRegionVersion = 3;

static void readAlphaTex(BinReader& reader)
{
    uint32_t numAlphaTex = reader.readInt();

    for(uint32_t i = 0; i < numAlphaTex; i++)
    {
        uint32_t tcode = reader.readInt();
        assert(tcode == 8 || tcode == 9 || tcode == 10);

        uint32_t texId = reader.readInt();
        assert((texId & 0xFF000000) == 0x05000000);
    }
}

static void read(BinReader& reader, Region::SceneType& sceneType)
{
    /*uint32_t sceneTypeUnk = */reader.readInt();

    uint32_t sceneCount = reader.readInt();
    sceneType.scenes.resize(sceneCount);

    for(ResourcePtr& scene : sceneType.scenes)
    {
        uint32_t resourceId = reader.readInt();
        assert((resourceId & 0xFF000000) == 0x12000000);

        scene = Core::get().resourceCache().get(resourceId);
    }
}

static void read(BinReader& reader, Region::TerrainType& terrainType)
{
    string terrainName = reader.readString();
    /*uint32_t terrainColor = */reader.readInt();

    uint32_t numSceneTypes = reader.readInt();
    terrainType.sceneTypes.resize(numSceneTypes);

    for(uint32_t& sceneType : terrainType.sceneTypes)
    {
        sceneType = reader.readInt();
    }
}

static void read(BinReader& reader, Region::TerrainTex& terrainTex)
{
    terrainTex.resourceId = reader.readInt();
    assert((terrainTex.resourceId & 0xFF000000) == 0x05000000);

    /*uint32_t texTiling = */reader.readInt();

    uint32_t maxVertBright = reader.readInt();
    uint32_t minVertBright = reader.readInt();
    assert(maxVertBright >= minVertBright);

    uint32_t maxVertSaturate = reader.readInt();
    uint32_t minVertSaturate = reader.readInt();
    assert(maxVertSaturate >= minVertSaturate);

    uint32_t maxVertHue = reader.readInt();
    uint32_t minVertHue = reader.readInt();
    assert(maxVertHue >= minVertHue);

    /*uint32_t detailTexTiling = */reader.readInt();

    uint32_t detailTexId = reader.readInt();
    assert((detailTexId & 0xFF000000) == 0x05000000);
}

Region::Region(uint32_t id, const void* data, size_t size) : ResourceImpl{id}
{
    BinReader reader(data, size);

    uint32_t resourceId = reader.readInt();
    assert(resourceId == id);

    uint32_t regionNumber = reader.readInt();
    assert(regionNumber == 1);

    uint32_t regionVersion = reader.readInt();
    assert(regionVersion == kRegionVersion);

    string regionName = reader.readString();
    assert(regionName == "Dereth");

    uint32_t unk1 = reader.readInt();
    assert(unk1 == 0xFF);

    uint32_t unk2 = reader.readInt();
    assert(unk2 == 0xFF);

    for(uint32_t i = 0; i < 0xFF; i++)
    {
        reader.readInt();
    }

    reader.readRaw(56);

    uint32_t numHours = reader.readInt();

    for(uint32_t i = 0; i < numHours; i++)
    {
        float startTime = reader.readFloat();
        assert(startTime >= 0.0 && startTime <= 1.0);

        uint32_t nightTime = reader.readInt();
        assert(nightTime == 0 || nightTime == 1);

        string hourName = reader.readString();
    }

    uint32_t numHolidays = reader.readInt();

    for(uint32_t i = 0; i < numHolidays; i++)
    {
        string holidayName = reader.readString();
    }

    uint32_t numSeasons = reader.readInt();

    for(uint32_t i = 0; i < numSeasons; i++)
    {
        uint32_t startDay = reader.readInt();
        assert(startDay <= 365);

        string seasonName = reader.readString();
    }

    reader.readRaw(49400);

    // AC: CSceneDesc
    uint32_t numSceneTypes = reader.readInt();
    sceneTypes.resize(numSceneTypes);

    for(SceneType& sceneType : sceneTypes)
    {
        read(reader, sceneType);
    }

    // AC: CTerrainDesc
    uint32_t numTerrainTypes = reader.readInt();
    terrainTypes.resize(numTerrainTypes);

    for(TerrainType& terrainType : terrainTypes)
    {
        read(reader, terrainType);
    }

    uint32_t unk3 = reader.readInt();
    assert(unk3 == 0);

    uint32_t unk4 = reader.readInt();
    assert(unk4 == 0x400);

    readAlphaTex(reader);
    readAlphaTex(reader);
    readAlphaTex(reader);

    uint32_t numTerrainTex = reader.readInt();
    terrainTextures.resize(numTerrainTex);

    for(uint32_t i = 0; i < numTerrainTex; i++)
    {
        uint32_t terrainTexNum = reader.readInt();
        assert(terrainTexNum == i);

        read(reader, terrainTextures[i]);
    }

    uint32_t unk5 = reader.readInt();
    assert(unk5 == 1);

    /*uint32_t smallMap = */reader.readInt();
    /*uint32_t largeMap = */reader.readInt();

    reader.readRaw(12);
    reader.assertEnd();
}
