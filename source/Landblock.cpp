#include "Landblock.h"
#include <algorithm>

const double Landblock::SQUARE_SIZE = 24.0;
const double Landblock::LANDBLOCK_SIZE = (Landblock::GRID_SIZE - 1) * Landblock::SQUARE_SIZE;

Landblock::Landblock(const void* data, size_t length)
{
    if(length != sizeof(RawData))
    {
        throw runtime_error("Bad landblock data length");
    }

    memcpy(&_data, data, sizeof(_data));

    buildOffsetMap();
}

const Landblock::RawData& Landblock::getRawData() const
{
    return _data;
}

double Landblock::getHeight(double x, double y) const
{
    assert(x >= 0.0 && x <= LANDBLOCK_SIZE);
    assert(y >= 0.0 && y <= LANDBLOCK_SIZE);

    double dix;
    auto fx = modf(x / SQUARE_SIZE, &dix);
    auto ix = (int)dix;

    double diy; 
    auto fy = modf(y / SQUARE_SIZE, &diy);
    auto iy = (int)diy;

    auto w1 = 0.0;
    auto w2 = 0.0;
    auto w3 = 0.0;
    auto w4 = 0.0;

    if(splitNESW(ix, iy))
    {
        // 4---3
        // |\  |
        // | \ |
        // |  \|
        // 1---2
        if(fy > 1.0 - fx)
        {
            // upper right half
            w2 = 1.0;
            w3 = 1.0;
            w4 = 1.0;
        }
        else
        {
            // lower left half
            w1 = 1.0;
            w2 = 1.0;
            w4 = 1.0;
        }
    }
    else
    {
        // 4---3
        // |  /|
        // | / |
        // |/  |
        // 1---2
        if(fy > fx)
        {
            // upper left half
            w1 = 1.0;
            w3 = 1.0;
            w4 = 1.0;
        }
        else
        {
            // lower right half
            w1 = 1.0;
            w2 = 1.0;
            w3 = 1.0;
        }
    }

    return (w1 * _data.heights[ix][iy] * (1.0 - fx) * (1.0 - fy) +
            w2 * _data.heights[ix + 1][iy] * fx * (1.0 - fy) +
            w3 * _data.heights[ix + 1][iy + 1] * fx * fy +
            w4 * _data.heights[ix][iy + 1] * (1.0 - fx) * fy) * 2.0;
}

const uint16_t* Landblock::getOffsetMap() const
{
    return _offsetMap.data();
}

int Landblock::getOffsetMapSize() const
{
    return OFFSET_MAP_SIZE;
}

double Landblock::getOffsetMapBase() const
{
    return _offsetMapBase;
}

double Landblock::getOffsetMapScale() const
{
    return _offsetMapScale;
}

bool Landblock::splitNESW(int x, int y) const
{
    // credits to Akilla
    uint32_t tx = ((_data.fileId >> 24) & 0xFF) * 8 + x;
    uint32_t ty = ((_data.fileId >> 16) & 0xFF) * 8 + y;
    uint32_t v = tx * ty * 0x0CCAC033 - tx * 0x421BE3BD + ty * 0x6C1AC587 - 0x519B8F25;
    return v & 0x80000000;
}

unique_ptr<Destructable>& Landblock::renderData()
{
    return _renderData;
}

static double cubic(double p[4], double x)
{
    return p[1] + 0.5 * x * (p[2] - p[0] + x * (2.0 * p[0] - 5.0 * p[1] + 4.0 * p[2] - p[3] + x * (3.0 * (p[1] - p[2]) + p[3] - p[0])));
}

static double bicubic(double p[4][4], double x, double y)
{
    double arr[4];
    arr[0] = cubic(p[0], y);
    arr[1] = cubic(p[1], y);
    arr[2] = cubic(p[2], y);
    arr[3] = cubic(p[3], y);
    return cubic(arr, x);
}

void Landblock::buildOffsetMap()
{
    static const int sampleSize = GRID_SIZE * 8;
    vector<double> sample(sampleSize * sampleSize);

    for(auto sy = 0; sy < sampleSize; sy++)
    {
        for(auto sx = 0; sx < sampleSize; sx++)
        {
            auto lx = double(sx) / double(sampleSize - 1) * LANDBLOCK_SIZE;
            auto ly = double(sy) / double(sampleSize - 1) * LANDBLOCK_SIZE;
            sample[sx + sy * sampleSize] = getHeight(lx, ly);
        }
    }

    vector<double> resample(OFFSET_MAP_SIZE * OFFSET_MAP_SIZE);

#define CLAMP(x, l, h) min(max(x, l), h)

    auto minOffset = numeric_limits<double>::max();
    auto maxOffset = numeric_limits<double>::min();

    for(auto oy = 0; oy < OFFSET_MAP_SIZE; oy++)
    {
        for(auto ox = 0; ox < OFFSET_MAP_SIZE; ox++)
        {
            auto sx = double(ox) / double(OFFSET_MAP_SIZE - 1) * double(sampleSize - 1);
            auto sy = double(oy) / double(OFFSET_MAP_SIZE - 1) * double(sampleSize - 1);

            auto ix = (int)sx;
            auto iy = (int)sy;

            auto fx = sx - ix;
            auto fy = sy - iy;

            double p[4][4];

            for(auto py = 0; py < 4; py++)
            {
                for(auto px = 0; px < 4; px++)
                {
                    p[px][py] = sample[CLAMP(ix + px - 1, 0, sampleSize - 1) + CLAMP(iy + py - 1, 0, sampleSize - 1) * sampleSize];
                }
            }

            auto lx = double(ox) / double(OFFSET_MAP_SIZE - 1) * LANDBLOCK_SIZE;
            auto ly = double(oy) / double(OFFSET_MAP_SIZE - 1) * LANDBLOCK_SIZE;

            double offset = bicubic(p, fx, fy) - getHeight(lx, ly);

            minOffset = min(minOffset, offset);
            maxOffset = max(maxOffset, offset);

            resample[ox + oy * OFFSET_MAP_SIZE] = offset;
        }
    }

#undef CLAMP

    _offsetMap.resize(OFFSET_MAP_SIZE * OFFSET_MAP_SIZE);
    _offsetMapBase = minOffset;
    _offsetMapScale = maxOffset - minOffset;

    for(auto oy = 0; oy < OFFSET_MAP_SIZE; oy++)
    {
        for(auto ox = 0; ox < OFFSET_MAP_SIZE; ox++)
        {
            double offset = resample[ox + oy * OFFSET_MAP_SIZE];
            _offsetMap[ox + (OFFSET_MAP_SIZE - oy - 1) * OFFSET_MAP_SIZE] = (offset - _offsetMapBase) / _offsetMapScale * double(0xFFFF);
        }
    }
}
