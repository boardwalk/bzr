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
#include "Land.h"
#include "resource/Region.h"
#include "resource/Scene.h"
#include "BinReader.h"
#include "Core.h"
#include "DatFile.h"
#include "LandcellManager.h"
#include "PRNG.h"
#include "ResourceCache.h"
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>

const fp_t Land::kCellSize = fp_t(24.0);
const fp_t Land::kBlockSize = fp_t(192.0);

static fp_t cubic(fp_t p[4], fp_t x)
{
    return p[1] + fp_t(0.5) * x * (p[2] - p[0] + x * (fp_t(2.0) * p[0] - fp_t(5.0) * p[1] + fp_t(4.0) * p[2] - p[3] + x * (fp_t(3.0) * (p[1] - p[2]) + p[3] - p[0])));
}

static fp_t bicubic(fp_t p[4][4], fp_t x, fp_t y)
{
    fp_t arr[]
    {
        cubic(p[0], y),
        cubic(p[1], y),
        cubic(p[2], y),
        cubic(p[3], y)
    };

    return cubic(arr, x);
}

Land::Land(const void* data, size_t size) : numStructures_(0)
{
    if(size != sizeof(Data))
    {
        throw runtime_error("Bad land data length");
    }

    memcpy(&data_, data, sizeof(data_));

    const Region& region = Core::get().region();

    for(int y = 0; y < kGridSize; y++)
    {
        for(int x = 0; x < kGridSize; x++)
        {
            heights_[x][y] = region.landHeights[data_.heightIndices[x][y]];
        }
    }
}

void Land::init()
{
    if(!offsetMap_.empty())
    {
        return;
    }

    if(data_.flags)
    {
        initStaticObjects();
    }

    initScenes();

    // sample another two times at the edges so we don't have to clamp our bicubic resample
    static const int sampleSize = kGridSize;
    static const int edgeSize = 2;
    static const int totalSampleSize = sampleSize + edgeSize * 2;

    vector<fp_t> sample(totalSampleSize * totalSampleSize);

    for(int sy = 0; sy < totalSampleSize; sy++)
    {
        for(int sx = 0; sx < totalSampleSize; sx++)
        {
            fp_t lx = static_cast<fp_t>(sx - edgeSize) / static_cast<fp_t>(sampleSize - 1) * kBlockSize;
            fp_t ly = static_cast<fp_t>(sy - edgeSize) / static_cast<fp_t>(sampleSize - 1) * kBlockSize;
            sample[sx + sy * totalSampleSize] = calcHeightUnbounded(lx, ly);
        }
    }

    vector<fp_t> resample(kOffsetMapSize * kOffsetMapSize);

    fp_t minOffset = numeric_limits<fp_t>::max();
    fp_t maxOffset = numeric_limits<fp_t>::min();

    for(int oy = 0; oy < kOffsetMapSize; oy++)
    {
        for(int ox = 0; ox < kOffsetMapSize; ox++)
        {
            fp_t sx = static_cast<fp_t>(ox) / static_cast<fp_t>(kOffsetMapSize - 1) * static_cast<fp_t>(sampleSize - 1);
            fp_t sy = static_cast<fp_t>(oy) / static_cast<fp_t>(kOffsetMapSize - 1) * static_cast<fp_t>(sampleSize - 1);

            int ix = static_cast<int>(sx);
            int iy = static_cast<int>(sy);

            fp_t fx = sx - ix;
            fp_t fy = sy - iy;

            fp_t p[4][4];

            for(int py = 0; py < 4; py++)
            {
                for(int px = 0; px < 4; px++)
                {
                    p[px][py] = sample[(edgeSize + ix + px - 1) + (edgeSize + iy + py - 1) * totalSampleSize];
                }
            }

            fp_t lx = static_cast<fp_t>(ox) / static_cast<fp_t>(kOffsetMapSize - 1) * kBlockSize;
            fp_t ly = static_cast<fp_t>(oy) / static_cast<fp_t>(kOffsetMapSize - 1) * kBlockSize;

            fp_t offset = bicubic(p, fx, fy) - calcHeight(lx, ly);

            minOffset = min(minOffset, offset);
            maxOffset = max(maxOffset, offset);

            resample[ox + oy * kOffsetMapSize] = offset;
        }
    }

    offsetMap_.resize(kOffsetMapSize * kOffsetMapSize);
    offsetMapBase_ = minOffset;
    offsetMapScale_ = maxOffset - minOffset;

    if(offsetMapScale_ < 0.0001)
    {
        memset(offsetMap_.data(), 0, offsetMap_.size() * sizeof(uint16_t));
    }
    else
    {
        for(int oy = 0; oy < kOffsetMapSize; oy++)
        {
            for(int ox = 0; ox < kOffsetMapSize; ox++)
            {
                fp_t offset = resample[ox + oy * kOffsetMapSize];
                offsetMap_[ox + oy * kOffsetMapSize] = static_cast<uint16_t>((offset - offsetMapBase_) / offsetMapScale_ * fp_t(0xFFFF));
            }
        }
    }

    normalMap_.resize(kOffsetMapSize * kOffsetMapSize * 3);

    for(int oy = 0; oy < kOffsetMapSize; oy++)
    {
        for(int ox = 0; ox < kOffsetMapSize; ox++)
        {
            int ox1 = max(ox - 1, 0);
            int oy1 = max(oy - 1, 0);

            int ox2 = min(ox + 1, kOffsetMapSize - 1);
            int oy2 = min(oy + 1, kOffsetMapSize - 1);

            fp_t lx1 = static_cast<fp_t>(ox1) / static_cast<fp_t>(kOffsetMapSize - 1) * kBlockSize;
            fp_t lx2 = static_cast<fp_t>(ox2) / static_cast<fp_t>(kOffsetMapSize - 1) * kBlockSize;
            fp_t ly1 = static_cast<fp_t>(oy1) / static_cast<fp_t>(kOffsetMapSize - 1) * kBlockSize;
            fp_t ly2 = static_cast<fp_t>(oy2) / static_cast<fp_t>(kOffsetMapSize - 1) * kBlockSize;

            fp_t h1 = resample[ox1 + oy1 * kOffsetMapSize] + calcHeight(lx1, ly1);
            fp_t h2 = resample[ox2 + oy1 * kOffsetMapSize] + calcHeight(lx2, ly1);
            fp_t h3 = resample[ox1 + oy2 * kOffsetMapSize] + calcHeight(lx1, ly2);

            glm::vec3 a{lx2 - lx1, 0.0, h2 - h1};
            glm::vec3 b{0.0, ly2 - ly1, h3 - h1};

            glm::vec3 n = glm::normalize(glm::cross(a, b)) * fp_t(0.5) + glm::vec3{0.5, 0.5, 0.5};
            normalMap_[(ox + oy * kOffsetMapSize) * 3] = static_cast<uint8_t>(n.x * fp_t(0xFF));
            normalMap_[(ox + oy * kOffsetMapSize) * 3 + 1] = static_cast<uint8_t>(n.y * fp_t(0xFF));
            normalMap_[(ox + oy * kOffsetMapSize) * 3 + 2] = static_cast<uint8_t>(n.z * fp_t(0xFF));
        }
    }
}

fp_t Land::getHeight(int gridX, int gridY) const
{
    return heights_[gridX][gridY];
}

uint8_t Land::getRoad(int gridX, int gridY) const
{
    return static_cast<uint8_t>(data_.styles[gridX][gridY] & 0x3);
}

uint8_t Land::getTerrain(int gridX, int gridY) const
{
    return static_cast<uint8_t>((data_.styles[gridX][gridY] >> 2) & 0x1f);
}

uint8_t Land::getTerrainScene(int gridX, int gridY) const
{
    return static_cast<uint8_t>(data_.styles[gridX][gridY] >> 11);
}

bool Land::isSplitNESW(int gridX, int gridY) const
{
    // credits to Akilla
    uint32_t cell_x = id().x() * 8 + gridX;
    uint32_t cell_y = id().y() * 8 + gridY;
    return prng(cell_x, cell_y, RND_MID_DIAG) >= 0.5;
}

Plane Land::calcPlane(fp_t x, fp_t y) const
{
    assert(x >= 0.0 && x <= kBlockSize);
    assert(y >= 0.0 && y <= kBlockSize);

    fp_t dix;
    fp_t fx = modf(x / kCellSize, &dix);
    int ix = static_cast<int>(dix);

    fp_t diy;
    fp_t fy = modf(y / kCellSize, &diy);
    int iy = static_cast<int>(diy);

    glm::vec3 v1(ix * kCellSize, iy * kCellSize, getHeight(ix, iy));
    glm::vec3 v2((ix + 1) * kCellSize, iy * kCellSize, getHeight(min(ix + 1, kGridSize - 1), iy));
    glm::vec3 v3(ix * kCellSize, (iy + 1) * kCellSize, getHeight(ix, min(iy + 1, kGridSize - 1)));
    glm::vec3 v4((ix + 1) * kCellSize, (iy + 1) * kCellSize, getHeight(min(ix + 1, kGridSize - 1), min(iy + 1, kGridSize - 1)));

    if(isSplitNESW(ix, iy))
    {
        // 3---4
        // |\  |
        // | \ |
        // |  \|
        // 1---2
        if(fy > 1.0 - fx)
        {
            // upper right half
            return Plane(v2, v4, v3);
        }
        else
        {
            // lower left half
            return Plane(v1, v2, v3);
        }
    }
    else
    {
        // 3---4
        // |  /|
        // | / |
        // |/  |
        // 1---2
        if(fy > fx)
        {
            // upper left half
            return Plane(v1, v4, v3);
        }
        else
        {
            // lower right half
            return Plane(v1, v2, v4);
        }
    }
}

fp_t Land::calcHeight(fp_t x, fp_t y) const
{
    return calcPlane(x, y).calcZ(x, y);
}

fp_t Land::calcHeightUnbounded(fp_t x, fp_t y) const
{
    int lx = id().x();
    int ly = id().y();

    while(x < 0.0)
    {
        lx--;
        x += kBlockSize;
    }

    while(x >= kBlockSize)
    {
        lx++;
        x -= kBlockSize;
    }

    while(y < 0.0)
    {
        ly--;
        y += kBlockSize;
    }

    while(y >= kBlockSize)
    {
        ly++;
        y -= kBlockSize;
    }

    if(lx < 0x00 || lx > 0xFF || ly < 0x00 || ly > 0xFF)
    {
        return fp_t(0.0);
    }

    auto it = Core::get().landcellManager().find(LandcellId(lx, ly));

    if(it == Core::get().landcellManager().end())
    {
        return fp_t(0.0);
    }

    Land& land = static_cast<Land&>(*it->second);

    return land.calcHeight(x, y);
}

LandcellId Land::id() const
{
    return LandcellId(data_.fileId);
}

uint32_t Land::numStructures() const
{
    return numStructures_;
}

const uint8_t* Land::normalMap() const
{
    return normalMap_.data();
}

void Land::initStaticObjects()
{
    // AC: CLandBlockInfo
    vector<uint8_t> blob = Core::get().cellDat().read(data_.fileId - 1);

    BinReader reader(blob.data(), blob.size());

    uint32_t fid = reader.readInt();
    assert(fid == data_.fileId - 1);
    UNUSED(fid);

    numStructures_ = reader.readInt();

    uint16_t numStaticObjects = reader.readShort();

    uint16_t unk1 = reader.readShort();
    assert(unk1 == 0);
    UNUSED(unk1);

    staticObjects_.resize(numStaticObjects);

    for(auto& staticObject : staticObjects_)
    {
        read(reader, staticObject);
    }

    uint16_t numStaticObjectsEx = reader.readShort();

    // I don't know what this is, but it means there's more data
    uint16_t unk2 = reader.readShort();
    assert(unk2 == 0 || unk2 == 1);
    UNUSED(unk2);

    staticObjects_.resize(numStaticObjects + numStaticObjectsEx);

    for(uint16_t i = 0; i < numStaticObjectsEx; i++)
    {
        // AC: BuildInfo
        read(reader, staticObjects_[numStaticObjects + i]);

        /*uint32_t numLeaves = */reader.readInt();
        uint32_t numPortals = reader.readInt();

        for(uint32_t pi = 0; pi < numPortals; pi++)
        {
            // AC: CBldPortal
            reader.readInt();
            reader.readShort();
            uint16_t numVisible = reader.readShort();

            for(uint16_t vi = 0; vi < numVisible; vi++)
            {
                /*cellId*/ reader.readShort();
            }

            reader.align();
        }
    }
}

void Land::initScenes()
{
    const Region& region = Core::get().region();

    for(int x = 0; x < kGridSize; x++)
    {
        for(int y = 0; y < kGridSize; y++)
        {
            uint32_t terrainTypeNum = getTerrain(x, y);
            uint32_t terrainSceneTypeNum = getTerrainScene(x, y);

            assert(terrainTypeNum < region.terrainTypes.size());
            const vector<uint32_t>& terrainSceneTypes = region.terrainTypes[terrainTypeNum].sceneTypes;

            assert(terrainSceneTypeNum < terrainSceneTypes.size());
            uint32_t sceneTypeNum = terrainSceneTypes[terrainSceneTypeNum];

            const Region::SceneType& sceneType = region.sceneTypes[sceneTypeNum];

            if(sceneType.scenes.empty())
            {
                continue;
            }

            uint32_t cellX = id().x() * 8 + x;
            uint32_t cellY = id().y() * 8 + y;
            uint32_t sceneNum = static_cast<uint32_t>(prng(cellX, cellY, RND_SCENE_PICK) * static_cast<double>(sceneType.scenes.size()));

            if(sceneNum >= sceneType.scenes.size())
            {
                sceneNum = 0;
            }

            const Scene& scene = sceneType.scenes[sceneNum]->cast<Scene>();

            initScene(x, y, scene);
        }
    }
}

void Land::initScene(int x, int y, const Scene& scene)
{
    uint32_t cellX = id().x() * 8 + x;
    uint32_t cellY = id().y() * 8 + y;

    double sceneRot = prng(cellX, cellY, RND_SCENE_ROT);

    for(uint32_t i = 0; i < scene.objects.size(); i++)
    {
        const Scene::ObjectDesc& objectDesc = scene.objects[i];

        if(objectDesc.isWeenieObj)
        {
            // not sure what these are for
            // they don't have a valid resourceId
            continue;
        }

        if(prng(cellX, cellY, RND_SCENE_FREQ + i) >= objectDesc.frequency)
        {
            // object hidden
            continue;
        }

        // calculate position within block
        glm::vec3 cellPos = objectDesc.position;
        
        if(objectDesc.displace.x > 0.0)
        {
            cellPos.x += static_cast<fp_t>(prng(cellX, cellY, RND_SCENE_DISP_X + i) * objectDesc.displace.x);
        }

        if(objectDesc.displace.y > 0.0)
        {
            cellPos.y += static_cast<fp_t>(prng(cellX, cellY, RND_SCENE_DISP_Y + i) * objectDesc.displace.y);
        }

        glm::vec3 tempPos = cellPos;

        if(sceneRot >= 0.75)
        {
            cellPos.x = tempPos.y;
            cellPos.y = -tempPos.x;
        }
        else if(sceneRot >= 0.5)
        {
            cellPos.x = -tempPos.x;
            cellPos.y = -tempPos.y;
        }
        else if(sceneRot >= 0.25)
        {
            cellPos.x = -tempPos.y;
            cellPos.y = tempPos.x;
        }
        else
        {
            cellPos.x = tempPos.y;
            cellPos.y = tempPos.x;
        }

        glm::vec3 blockPos = cellPos + glm::vec3(x * kCellSize, y * kCellSize, 0.0);

        if(blockPos.x < 0.0 || blockPos.x >= kBlockSize || blockPos.y < 0.0 || blockPos.y >= kBlockSize)
        {
            continue;
        }

        if(roadAtPoint(blockPos.x, blockPos.y))
        {
            continue;
        }

        Plane landPlane = calcPlane(blockPos.x, blockPos.y);

        if(landPlane.normal.z < objectDesc.minSlope || landPlane.normal.z > objectDesc.maxSlope)
        {
            continue;
        }

        blockPos.z += landPlane.calcZ(blockPos.x, blockPos.y);

        // calculate scale
        fp_t scale = static_cast<fp_t>(objectDesc.minScale * pow(objectDesc.maxScale / objectDesc.minScale, prng(cellX, cellY, RND_SCENE_SCALE1 + i)));

        // calculate rotation
        fp_t randRot = static_cast<fp_t>(prng(cellX, cellY, RND_SCENE_OBJROT + i)) * glm::radians(objectDesc.maxRotation);
        glm::quat rotation = glm::angleAxis(randRot, glm::vec3(0.0, 0.0, 1.0)) * objectDesc.rotation;

        glm::mat4 translateMat = glm::translate(glm::mat4{}, blockPos);
        glm::mat4 rotateMat = glm::mat4_cast(rotation);
        glm::mat4 scaleMat = glm::scale(glm::mat4{}, glm::vec3{scale, scale, scale});

        // add static object
        StaticObject staticObject;
        staticObject.resource = Core::get().resourceCache().get(objectDesc.resourceId);
        staticObject.transform = translateMat * rotateMat * scaleMat;
        staticObjects_.push_back(staticObject);
    }
}

bool Land::roadAtPoint(fp_t x, fp_t y) const
{
    assert(x >= 0.0 && x < kBlockSize);
    assert(y >= 0.0 && y < kBlockSize);

    const fp_t kRoadHalfWidth = fp_t(5.0);

    fp_t dix;
    fp_t fx = modf(x / kCellSize, &dix);
    int ix = static_cast<int>(dix);

    fp_t diy;
    fp_t fy = modf(y / kCellSize, &diy);
    int iy = static_cast<int>(diy);

    int r1 = getRoad(ix, iy);
    int r2 = getRoad(ix + 1, iy);
    int r3 = getRoad(ix, iy + 1);
    int r4 = getRoad(ix + 1, iy + 1);

    if(r1 && r2 && r3 && r4)
    {
        return true;
    }

    if(r1)
    {
        if(r2 && fy < kRoadHalfWidth)
        {
            return true;
        }

        if(r3 && fx < kRoadHalfWidth)
        {
            return true;
        }

        if(fx < kRoadHalfWidth && fy < kRoadHalfWidth)
        {
            return true;
        }
    }

    if(r2)
    {
        if(r4 && fx > kCellSize - kRoadHalfWidth)
        {
            return true;
        }

        if(fx > kCellSize - kRoadHalfWidth && fy < kRoadHalfWidth)
        {
            return true;
        }
    }

    if(r3)
    {
        if(r4 && fy > kCellSize - kRoadHalfWidth)
        {
            return true;
        }

        if(fx < kRoadHalfWidth && fy > kCellSize - kRoadHalfWidth)
        {
            return true;
        }
    }

    if(r4)
    {
        if(fx > kCellSize - kRoadHalfWidth && fy > kCellSize - kRoadHalfWidth)
        {
            return true;
        }
    }

    // TODO diagonals?

    return false;
}
