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

Land::Land(const void* data, size_t size) : _numStructures(0)
{
    if(size != sizeof(Data))
    {
        throw runtime_error("Bad land data length");
    }

    memcpy(&_data, data, sizeof(_data));
}

void Land::init()
{
    if(!_offsetMap.empty())
    {
        return;
    }

    if(_data.flags)
    {
        initDoodads();
    }

    // sample another two times at the edges so we don't have to clamp our bicubic resample
    static const int sampleSize = GRID_SIZE;
    static const int edgeSize = 2;
    static const int totalSampleSize = sampleSize + edgeSize * 2;

    vector<fp_t> sample(totalSampleSize * totalSampleSize);

    for(auto sy = 0; sy < totalSampleSize; sy++)
    {
        for(auto sx = 0; sx < totalSampleSize; sx++)
        {
            auto lx = fp_t(sx - edgeSize) / fp_t(sampleSize - 1) * BLOCK_SIZE;
            auto ly = fp_t(sy - edgeSize) / fp_t(sampleSize - 1) * BLOCK_SIZE;
            sample[sx + sy * totalSampleSize] = calcHeightUnbounded(lx, ly);
        }
    }

    vector<fp_t> resample(OFFSET_MAP_SIZE * OFFSET_MAP_SIZE);

    auto minOffset = numeric_limits<fp_t>::max();
    auto maxOffset = numeric_limits<fp_t>::min();

    for(auto oy = 0; oy < OFFSET_MAP_SIZE; oy++)
    {
        for(auto ox = 0; ox < OFFSET_MAP_SIZE; ox++)
        {
            auto sx = fp_t(ox) / fp_t(OFFSET_MAP_SIZE - 1) * fp_t(sampleSize - 1);
            auto sy = fp_t(oy) / fp_t(OFFSET_MAP_SIZE - 1) * fp_t(sampleSize - 1);

            auto ix = (int)sx;
            auto iy = (int)sy;

            auto fx = sx - ix;
            auto fy = sy - iy;

            fp_t p[4][4];

            for(auto py = 0; py < 4; py++)
            {
                for(auto px = 0; px < 4; px++)
                {
                    p[px][py] = sample[(edgeSize + ix + px - 1) + (edgeSize + iy + py - 1) * totalSampleSize];
                }
            }

            auto lx = fp_t(ox) / fp_t(OFFSET_MAP_SIZE - 1) * BLOCK_SIZE;
            auto ly = fp_t(oy) / fp_t(OFFSET_MAP_SIZE - 1) * BLOCK_SIZE;

            auto offset = bicubic(p, fx, fy) - calcHeight(lx, ly);

            minOffset = min(minOffset, offset);
            maxOffset = max(maxOffset, offset);

            resample[ox + oy * OFFSET_MAP_SIZE] = offset;
        }
    }

    _offsetMap.resize(OFFSET_MAP_SIZE * OFFSET_MAP_SIZE);
    _offsetMapBase = minOffset;
    _offsetMapScale = maxOffset - minOffset;

    if(_offsetMapScale < 0.0001)
    {
        memset(_offsetMap.data(), 0, _offsetMap.size() * sizeof(uint16_t));
    }
    else
    {
        for(auto oy = 0; oy < OFFSET_MAP_SIZE; oy++)
        {
            for(auto ox = 0; ox < OFFSET_MAP_SIZE; ox++)
            {
                fp_t offset = resample[ox + oy * OFFSET_MAP_SIZE];
                _offsetMap[ox + oy * OFFSET_MAP_SIZE] = (uint16_t)((offset - _offsetMapBase) / _offsetMapScale * fp_t(0xFFFF));
            }
        }
    }

    _normalMap.resize(OFFSET_MAP_SIZE * OFFSET_MAP_SIZE * 3);

    for(auto oy = 0; oy < OFFSET_MAP_SIZE; oy++)
    {
        for(auto ox = 0; ox < OFFSET_MAP_SIZE; ox++)
        {
            auto ox1 = max(ox - 1, 0);
            auto oy1 = max(oy - 1, 0);

            auto ox2 = min(ox + 1, OFFSET_MAP_SIZE - 1);
            auto oy2 = min(oy + 1, OFFSET_MAP_SIZE - 1);

            auto lx1 = fp_t(ox1) / fp_t(OFFSET_MAP_SIZE - 1) * BLOCK_SIZE;
            auto lx2 = fp_t(ox2) / fp_t(OFFSET_MAP_SIZE - 1) * BLOCK_SIZE;
            auto ly1 = fp_t(oy1) / fp_t(OFFSET_MAP_SIZE - 1) * BLOCK_SIZE;
            auto ly2 = fp_t(oy2) / fp_t(OFFSET_MAP_SIZE - 1) * BLOCK_SIZE;

            fp_t h1 = resample[ox1 + oy1 * OFFSET_MAP_SIZE] + calcHeight(lx1, ly1);
            fp_t h2 = resample[ox2 + oy1 * OFFSET_MAP_SIZE] + calcHeight(lx2, ly1);
            fp_t h3 = resample[ox1 + oy2 * OFFSET_MAP_SIZE] + calcHeight(lx1, ly2);

            glm::vec3 a(lx2 - lx1, 0.0, h2 - h1);
            glm::vec3 b(0.0, ly2 - ly1, h3 - h1);

            auto n = glm::normalize(glm::cross(a, b)) * fp_t(0.5) + glm::vec3(0.5, 0.5, 0.5);
            _normalMap[(ox + oy * OFFSET_MAP_SIZE) * 3] = (uint8_t)(n.x * fp_t(0xFF));
            _normalMap[(ox + oy * OFFSET_MAP_SIZE) * 3 + 1] = (uint8_t)(n.y * fp_t(0xFF));
            _normalMap[(ox + oy * OFFSET_MAP_SIZE) * 3 + 2] = (uint8_t)(n.z * fp_t(0xFF));
        }
    }
}

fp_t Land::calcHeight(fp_t x, fp_t y) const
{
    assert(x >= 0.0 && x <= BLOCK_SIZE);
    assert(y >= 0.0 && y <= BLOCK_SIZE);

    fp_t dix;
    auto fx = modf(x / CELL_SIZE, &dix);
    auto ix = (int)dix;

    fp_t diy; 
    auto fy = modf(y / CELL_SIZE, &diy);
    auto iy = (int)diy;

    auto h1 = _data.heights[ix][iy] * fp_t(2.0);
    auto h2 = _data.heights[min(ix + 1, GRID_SIZE - 1)][iy] * fp_t(2.0);
    auto h3 = _data.heights[ix][min(iy + 1, GRID_SIZE - 1)] * fp_t(2.0);
    auto h4 = _data.heights[min(ix + 1, GRID_SIZE - 1)][min(iy + 1, GRID_SIZE - 1)] * fp_t(2.0);

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

    auto hb = h1 * (fp_t(1.0) - fx) + h2 * fx;
    auto ht = h3 * (fp_t(1.0) - fx) + h4 * fx;
    return hb * (fp_t(1.0) - fy) + ht * fy;
}

fp_t Land::calcHeightUnbounded(fp_t x, fp_t y) const
{
    auto thisId = id();

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
    return LandcellId(_data.fileId);
}

const Land::Data& Land::data() const
{
    return _data;
}

uint32_t Land::numStructures() const
{
    return _numStructures;
}

const uint8_t* Land::normalMap() const
{
    return _normalMap.data();
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
    auto blob = Core::get().cellDat().read(_data.fileId - 1);

    BinReader reader(blob.data(), blob.size());

    auto fid = reader.read<uint32_t>();
    assert(fid == _data.fileId - 1);
    
    _numStructures = reader.read<uint32_t>();

    auto numDoodads = reader.read<uint16_t>();
    _doodads.resize(numDoodads);

    auto unk1 = reader.read<uint16_t>();
    assert(unk1 == 0);

    for(auto di = 0u; di < numDoodads; di++)
    {
        _doodads[di].read(reader);
    }

    auto numDoodadsEx = reader.read<uint16_t>();
    _doodads.resize(numDoodads + numDoodadsEx);

    // I don't know what this is, but it means there's more data
    auto unk2 = reader.read<uint16_t>();    
    assert(unk2 == 0 || unk2 == 1);

    for(auto di = 0u; di < numDoodadsEx; di++)
    {
        _doodads[numDoodads + di].read(reader);

        reader.read<uint32_t>();
        auto numPortals = reader.read<uint32_t>();

        // credits to Akilla
        for(auto pi = 0u; pi < numPortals; pi++)
        {
            reader.read<uint32_t>();
            reader.read<uint16_t>();
            auto numVisible = reader.read<uint16_t>();

            for(auto vi = 0; vi < numVisible; vi++)
            {
                reader.read<uint16_t>(); // structure index
            }

            reader.align();
        }
    }
}
