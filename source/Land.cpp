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
#include "BinReader.h"
#include "Core.h"
#include "DatFile.h"
#include "LandcellManager.h"
#include <algorithm>

const fp_t Land::CELL_SIZE = fp_t(24.0);
const fp_t Land::BLOCK_SIZE = fp_t(192.0);

static fp_t cubic(fp_t p[4], fp_t x)
{
    return p[1] + fp_t(0.5) * x * (p[2] - p[0] + x * (fp_t(2.0) * p[0] - fp_t(5.0) * p[1] + fp_t(4.0) * p[2] - p[3] + x * (fp_t(3.0) * (p[1] - p[2]) + p[3] - p[0])));
}

static fp_t bicubic(fp_t p[4][4], fp_t x, fp_t y)
{
    fp_t arr[4];
    arr[0] = cubic(p[0], y);
    arr[1] = cubic(p[1], y);
    arr[2] = cubic(p[2], y);
    arr[3] = cubic(p[3], y);
    return cubic(arr, x);
}

Land::Land(const void* data, size_t size) : numStructures_(0)
{
    if(size != sizeof(Data))
    {
        throw runtime_error("Bad land data length");
    }

    memcpy(&data_, data, sizeof(data_));
}

void Land::init()
{
    if(!offsetMap_.empty())
    {
        return;
    }

    if(data_.flags)
    {
        initDoodads();
    }

    // sample another two times at the edges so we don't have to clamp our bicubic resample
    static const int sampleSize = GRID_SIZE;
    static const int edgeSize = 2;
    static const int totalSampleSize = sampleSize + edgeSize * 2;

    vector<fp_t> sample(totalSampleSize * totalSampleSize);

    for(int sy = 0; sy < totalSampleSize; sy++)
    {
        for(int sx = 0; sx < totalSampleSize; sx++)
        {
            fp_t lx = fp_t(sx - edgeSize) / fp_t(sampleSize - 1) * BLOCK_SIZE;
            fp_t ly = fp_t(sy - edgeSize) / fp_t(sampleSize - 1) * BLOCK_SIZE;
            sample[sx + sy * totalSampleSize] = calcHeightUnbounded(lx, ly);
        }
    }

    vector<fp_t> resample(OFFSET_MAP_SIZE * OFFSET_MAP_SIZE);

    fp_t minOffset = numeric_limits<fp_t>::max();
    fp_t maxOffset = numeric_limits<fp_t>::min();

    for(int oy = 0; oy < OFFSET_MAP_SIZE; oy++)
    {
        for(int ox = 0; ox < OFFSET_MAP_SIZE; ox++)
        {
            fp_t sx = fp_t(ox) / fp_t(OFFSET_MAP_SIZE - 1) * fp_t(sampleSize - 1);
            fp_t sy = fp_t(oy) / fp_t(OFFSET_MAP_SIZE - 1) * fp_t(sampleSize - 1);

            int ix = (int)sx;
            int iy = (int)sy;

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

            fp_t lx = fp_t(ox) / fp_t(OFFSET_MAP_SIZE - 1) * BLOCK_SIZE;
            fp_t ly = fp_t(oy) / fp_t(OFFSET_MAP_SIZE - 1) * BLOCK_SIZE;

            fp_t offset = bicubic(p, fx, fy) - calcHeight(lx, ly);

            minOffset = min(minOffset, offset);
            maxOffset = max(maxOffset, offset);

            resample[ox + oy * OFFSET_MAP_SIZE] = offset;
        }
    }

    offsetMap_.resize(OFFSET_MAP_SIZE * OFFSET_MAP_SIZE);
    offsetMapBase_ = minOffset;
    offsetMapScale_ = maxOffset - minOffset;

    if(offsetMapScale_ < 0.0001)
    {
        memset(offsetMap_.data(), 0, offsetMap_.size() * sizeof(uint16_t));
    }
    else
    {
        for(int oy = 0; oy < OFFSET_MAP_SIZE; oy++)
        {
            for(int ox = 0; ox < OFFSET_MAP_SIZE; ox++)
            {
                fp_t offset = resample[ox + oy * OFFSET_MAP_SIZE];
                offsetMap_[ox + oy * OFFSET_MAP_SIZE] = (uint16_t)((offset - offsetMapBase_) / offsetMapScale_ * fp_t(0xFFFF));
            }
        }
    }

    normalMap_.resize(OFFSET_MAP_SIZE * OFFSET_MAP_SIZE * 3);

    for(int oy = 0; oy < OFFSET_MAP_SIZE; oy++)
    {
        for(int ox = 0; ox < OFFSET_MAP_SIZE; ox++)
        {
            int ox1 = max(ox - 1, 0);
            int oy1 = max(oy - 1, 0);

            int ox2 = min(ox + 1, OFFSET_MAP_SIZE - 1);
            int oy2 = min(oy + 1, OFFSET_MAP_SIZE - 1);

            fp_t lx1 = fp_t(ox1) / fp_t(OFFSET_MAP_SIZE - 1) * BLOCK_SIZE;
            fp_t lx2 = fp_t(ox2) / fp_t(OFFSET_MAP_SIZE - 1) * BLOCK_SIZE;
            fp_t ly1 = fp_t(oy1) / fp_t(OFFSET_MAP_SIZE - 1) * BLOCK_SIZE;
            fp_t ly2 = fp_t(oy2) / fp_t(OFFSET_MAP_SIZE - 1) * BLOCK_SIZE;

            fp_t h1 = resample[ox1 + oy1 * OFFSET_MAP_SIZE] + calcHeight(lx1, ly1);
            fp_t h2 = resample[ox2 + oy1 * OFFSET_MAP_SIZE] + calcHeight(lx2, ly1);
            fp_t h3 = resample[ox1 + oy2 * OFFSET_MAP_SIZE] + calcHeight(lx1, ly2);

            glm::vec3 a(lx2 - lx1, 0.0, h2 - h1);
            glm::vec3 b(0.0, ly2 - ly1, h3 - h1);

            glm::vec3 n = glm::normalize(glm::cross(a, b)) * fp_t(0.5) + glm::vec3(0.5, 0.5, 0.5);
            normalMap_[(ox + oy * OFFSET_MAP_SIZE) * 3] = (uint8_t)(n.x * fp_t(0xFF));
            normalMap_[(ox + oy * OFFSET_MAP_SIZE) * 3 + 1] = (uint8_t)(n.y * fp_t(0xFF));
            normalMap_[(ox + oy * OFFSET_MAP_SIZE) * 3 + 2] = (uint8_t)(n.z * fp_t(0xFF));
        }
    }
}

fp_t Land::calcHeight(fp_t x, fp_t y) const
{
    assert(x >= 0.0 && x <= BLOCK_SIZE);
    assert(y >= 0.0 && y <= BLOCK_SIZE);

    fp_t dix;
    fp_t fx = modf(x / CELL_SIZE, &dix);
    int ix = (int)dix;

    fp_t diy;
    fp_t fy = modf(y / CELL_SIZE, &diy);
    int iy = (int)diy;

    fp_t h1 = data_.heights[ix][iy] * fp_t(2.0);
    fp_t h2 = data_.heights[min(ix + 1, GRID_SIZE - 1)][iy] * fp_t(2.0);
    fp_t h3 = data_.heights[ix][min(iy + 1, GRID_SIZE - 1)] * fp_t(2.0);
    fp_t h4 = data_.heights[min(ix + 1, GRID_SIZE - 1)][min(iy + 1, GRID_SIZE - 1)] * fp_t(2.0);

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
            h1 = h2 - (h4 - h3);
        }
        else
        {
            // lower left half
            h4 = h2 + (h3 - h1);
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
            h2 = h1 + (h4 - h3);
        }
        else
        {
            // lower right half
            h3 = h4 - (h2 - h1);
        }
    }

    fp_t hb = h1 * (fp_t(1.0) - fx) + h2 * fx;
    fp_t ht = h3 * (fp_t(1.0) - fx) + h4 * fx;
    return hb * (fp_t(1.0) - fy) + ht * fy;
}

fp_t Land::calcHeightUnbounded(fp_t x, fp_t y) const
{
    LandcellId thisId = id();

    while(x < 0.0)
    {
        thisId = LandcellId(thisId.x() - 1, thisId.y());
        x += BLOCK_SIZE;
    }

    while(x >= BLOCK_SIZE)
    {
        thisId = LandcellId(thisId.x() + 1, thisId.y());
        x -= BLOCK_SIZE;
    }

    while(y < 0.0)
    {
        thisId = LandcellId(thisId.x(), thisId.y() - 1);
        y += BLOCK_SIZE;
    }

    while(y >= BLOCK_SIZE)
    {
        thisId = LandcellId(thisId.x(), thisId.y() + 1);
        y -= BLOCK_SIZE;
    }

    auto it = Core::get().landcellManager().find(thisId);

    if(it == Core::get().landcellManager().end())
    {
        throw logic_error("landcell not found");
    }

    Land& land = static_cast<Land&>(*it->second);

    return land.calcHeight(x, y);
}

LandcellId Land::id() const
{
    return LandcellId(data_.fileId);
}

const Land::Data& Land::data() const
{
    return data_;
}

uint32_t Land::numStructures() const
{
    return numStructures_;
}

const uint8_t* Land::normalMap() const
{
    return normalMap_.data();
}

bool Land::isSplitNESW(int x, int y) const
{
    // credits to Akilla
    uint32_t tx = id().x() * 8 + x;
    uint32_t ty = id().y() * 8 + y;
    uint32_t v = tx * ty * 0x0CCAC033 - tx * 0x421BE3BD + ty * 0x6C1AC587 - 0x519B8F25;
    return (v & 0x80000000) != 0;
}

void Land::initDoodads()
{
    vector<uint8_t> blob = Core::get().cellDat().read(data_.fileId - 1);

    BinReader reader(blob.data(), blob.size());

    uint32_t fid = reader.read<uint32_t>();
    assert(fid == data_.fileId - 1);

    numStructures_ = reader.read<uint32_t>();

    uint16_t numDoodads = reader.read<uint16_t>();
    doodads_.resize(numDoodads);

    uint16_t unk1 = reader.read<uint16_t>();
    assert(unk1 == 0);

    for(uint16_t di = 0; di < numDoodads; di++)
    {
        doodads_[di].read(reader);
    }

    uint16_t numDoodadsEx = reader.read<uint16_t>();
    doodads_.resize(numDoodads + numDoodadsEx);

    // I don't know what this is, but it means there's more data
    uint16_t unk2 = reader.read<uint16_t>();
    assert(unk2 == 0 || unk2 == 1);

    for(uint16_t di = 0; di < numDoodadsEx; di++)
    {
        doodads_[numDoodads + di].read(reader);

        reader.read<uint32_t>();
        uint32_t numPortals = reader.read<uint32_t>();

        // credits to Akilla
        for(uint32_t pi = 0; pi < numPortals; pi++)
        {
            reader.read<uint32_t>();
            reader.read<uint16_t>();
            uint16_t numVisible = reader.read<uint16_t>();

            for(uint16_t vi = 0; vi < numVisible; vi++)
            {
                reader.read<uint16_t>(); // structure index
            }

            reader.align();
        }
    }
}
