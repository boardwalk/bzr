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

    buildHeightMap();
}

Landblock::Landblock(Landblock&& other)
{
    _data = other._data;
    _heightMap = move(other._heightMap);
    _heightMapBase = other._heightMapBase;
    _heightMapScale = other._heightMapScale;
    _renderData = move(other._renderData);
}

const Landblock::RawData& Landblock::rawData() const
{
    return _data;
}

double Landblock::calcHeight(double x, double y) const
{
    assert(x >= 0.0 && x <= LANDBLOCK_SIZE);
    assert(y >= 0.0 && y <= LANDBLOCK_SIZE);

    double dix;
    auto fx = modf(x / SQUARE_SIZE, &dix);
    auto ix = (int)dix;

    double diy; 
    auto fy = modf(y / SQUARE_SIZE, &diy);
    auto iy = (int)diy;

    double h1 = _data.heights[ix][iy] * 2.0;
    double h2 = _data.heights[min(ix + 1, GRID_SIZE - 1)][iy] * 2.0;
    double h3 = _data.heights[ix][min(iy + 1, GRID_SIZE - 1)] * 2.0;
    double h4 = _data.heights[min(ix + 1, GRID_SIZE - 1)][min(iy + 1, GRID_SIZE - 1)] * 2.0;

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

    auto hb = h1 * (1.0 - fx) + h2 * fx;
    auto ht = h3 * (1.0 - fx) + h4 * fx;
    return hb * (1.0 - fy) + ht * fy;
}

const uint16_t* Landblock::heightMap() const
{
    return _heightMap.data();
}

double Landblock::heightMapBase() const
{
    return _heightMapBase;
}

double Landblock::heightMapScale() const
{
    return _heightMapScale;
}

bool Landblock::isSplitNESW(int x, int y) const
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

void Landblock::buildHeightMap()
{
    static const int sampleSize = GRID_SIZE;
    vector<double> sample(sampleSize * sampleSize);

    for(auto sy = 0; sy < sampleSize; sy++)
    {
        for(auto sx = 0; sx < sampleSize; sx++)
        {
            auto lx = double(sx) / double(sampleSize - 1) * LANDBLOCK_SIZE;
            auto ly = double(sy) / double(sampleSize - 1) * LANDBLOCK_SIZE;
            sample[sx + sy * sampleSize] = calcHeight(lx, ly);
        }
    }

    vector<double> resample(HEIGHT_MAP_SIZE * HEIGHT_MAP_SIZE);

#define CLAMP(x, l, h) min(max(x, l), h)

    auto minHeight = numeric_limits<double>::max();
    auto maxHeight = numeric_limits<double>::min();

    for(auto hy = 0; hy < HEIGHT_MAP_SIZE; hy++)
    {
        for(auto hx = 0; hx < HEIGHT_MAP_SIZE; hx++)
        {
            auto sx = double(hx) / double(HEIGHT_MAP_SIZE - 1) * double(sampleSize - 1);
            auto sy = double(hy) / double(HEIGHT_MAP_SIZE - 1) * double(sampleSize - 1);

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

            auto height = bicubic(p, fx, fy);

            minHeight = min(minHeight, height);
            maxHeight = max(maxHeight, height);

            resample[hx + hy * HEIGHT_MAP_SIZE] = height;
        }
    }

#undef CLAMP

    _heightMap.resize(HEIGHT_MAP_SIZE * HEIGHT_MAP_SIZE);
    _heightMapBase = minHeight;
    _heightMapScale = maxHeight - minHeight;

    for(auto hy = 0; hy < HEIGHT_MAP_SIZE; hy++)
    {
        for(auto hx = 0; hx < HEIGHT_MAP_SIZE; hx++)
        {
            double height = resample[hx + hy * HEIGHT_MAP_SIZE];
            _heightMap[hx + hy * HEIGHT_MAP_SIZE] = (height - _heightMapBase) / _heightMapScale * double(0xFFFF);
        }
    }
}
