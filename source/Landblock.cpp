#include "Landblock.h"
#include <algorithm>

const double Landblock::SQUARE_SIZE = 24.0;
const double Landblock::LANDBLOCK_SIZE = (Landblock::GRID_SIZE - 1) * Landblock::SQUARE_SIZE;

static double getHeight(const double* data, size_t size, Vec2 point)
{
    assert(point.x >= 0.0 && point.x <= Landblock::LANDBLOCK_SIZE);
    assert(point.y >= 0.0 && point.y <= Landblock::LANDBLOCK_SIZE);

    double intx;
    auto fracx = modf(point.x / Landblock::LANDBLOCK_SIZE * size, &intx);
    auto x = (int)intx;

    double inty; 
    auto fracy = modf(point.y / Landblock::LANDBLOCK_SIZE * size, &inty);
    auto y = (int)inty;

    // 4---3
    // |  /|
    // | / |
    // |/  |
    // 1---2
    // or:
    // 4---3
    // |\  |
    // | \ |
    // |  \|
    // 1---2
    // we'll assume the first for now

    auto h1 = data[x + y * size];
    auto h2 = data[(x + 1) + y * size];
    auto h3 = data[(x + 1) + (y + 1) * size];
    auto h4 = data[x + (y + 1) * size];

    if(fracy > fracx)
    {
        // upper left half
        return
            h1 * (1.0 - fracx) * (1.0 - fracy) +
            h3 * fracx * fracy +
            h4 * (1.0 - fracx) * fracy;
    }
    else
    {
        // lower right half
        return
            h1 * (1.0 - fracx) * (1.0 - fracy) +
            h2 * fracx * (1.0 - fracy) +
            h3 * fracx * fracy;
    }
}

Landblock::Landblock(const void* data, size_t length)
{
    if(length != sizeof(RawData))
    {
        throw runtime_error("Bad landblock data length");
    }

    memcpy(&_data, data, sizeof(_data));

    _original.reset(new double[GRID_SIZE * GRID_SIZE]);

    for(auto x = 0; x < GRID_SIZE; x++)
    {
        for(auto y = 0; y < GRID_SIZE; y++)
        {
            _original[x + y * GRID_SIZE] = _data.heights[x][y] * 2.0;
        }
    }

    // TODO arbitrary, make this configurable
    subdivide(4);
}

const double* Landblock::getSubdividedData() const
{
    return _subdivided.get();
}

size_t Landblock::getSubdividedSize() const
{
    return GRID_SIZE * (1 << _nsubdivisions);
}

double Landblock::getOriginalHeight(Vec2 point) const
{
    return getHeight(_original.get(), GRID_SIZE, point);
}

double Landblock::getSubdividedHeight(Vec2 point) const
{
    return getHeight(_subdivided.get(), GRID_SIZE * (1 << _nsubdivisions), point);
}

unique_ptr<Destructable>& Landblock::renderData()
{
    return _renderData;
}

void Landblock::subdivide(int n)
{
    _nsubdivisions = 0;
    _subdivided.reset(new double[GRID_SIZE * GRID_SIZE]);
    memcpy(_subdivided.get(), _original.get(), sizeof(double) * GRID_SIZE * GRID_SIZE);

    while(_nsubdivisions < n)
    {
        subdivideOnce();
    }
}

// | / | / | / | /
// 3 - x - x - x -
// | / | / | / | /
// 2 - x - x - x -
// | / | / | / | /
// 1 - x - x - x -
// | / | / | / | /
// 0 - 1 - 2 - 3 -

// 8x8
// 4x4 original points
// 4x4 face points
// 4x4 horizontal edge points
// 4x4 vertical edge points

//
// Implements Catmull-Clark subdivision for landblock geometry
// Unfortunately we'd need the neighbor landblocks to not fudge the edges
// We'll see how this looks
// http://www.rorydriscoll.com/2008/08/01/catmull-clark-subdivision-the-basics/
//
void Landblock::subdivideOnce()
{
    auto size = GRID_SIZE * (1 << _nsubdivisions);
    auto newSize = size * 2;
    unique_ptr<double[]> newSubdivided(new double[newSize * newSize]);

    for(auto y = 0; y < size; y++)
    {
        for(auto x = 0; x < size; x++)
        {
            auto h1 = _subdivided[x + y * size];
            auto h2 = _subdivided[min(x + 1, size - 1) + y * size];
            auto h3 = _subdivided[min(x + 1, size - 1) + min(y + 1, size - 1) * size];
            auto h4 = _subdivided[x + min(y + 1, size - 1) * size];

            // copy original control point
            newSubdivided[(x * 2) + (y * 2) * newSize] = h1;

            // add horizontal edge point
            newSubdivided[(x * 2 + 1) + (y * 2) * newSize] = (h1 + h2) / 2.0;

            // add vertical edge point
            newSubdivided[(x * 2) + (y * 2 + 1) * newSize] = (h1 + h4) / 2.0;

            // add new face point
            newSubdivided[(x * 2 + 1) + (y * 2 + 1) * newSize] = (h1 + h2 + h3 + h4) / 4.0;
        }
    }

    for(auto y = 0; y < newSize; y += 2)
    {
        for(auto x = 0; x < newSize; x += 2)
        {
            // average adjacent face points
            auto f1 = newSubdivided[max(x - 1, 0) + max(y - 1, 0) * newSize];
            auto f2 = newSubdivided[min(x + 1, newSize - 1) + max(y - 1, 0) * newSize];
            auto f3 = newSubdivided[min(x + 1, newSize - 1) + min(y + 1, newSize - 1) * newSize];
            auto f4 = newSubdivided[max(x - 1, 0) + min(y + 1, newSize - 1) * newSize];
            auto f = (f1 + f2 + f3 + f4) / 4.0;

            // average adjacent edge points
            auto e1 = newSubdivided[max(x - 1, 0) + y * newSize];
            auto e2 = newSubdivided[min(x + 1, newSize - 1) + y * newSize];
            auto e3 = newSubdivided[x + max(y - 1, 0) * newSize];
            auto e4 = newSubdivided[x + min(y + 1, newSize - 1) * newSize];
            auto e = (e1 + e2 + e3 + e4) / 4.0;

            // grab existing control point
            auto c = newSubdivided[x + y * newSize];

            // update control point
            newSubdivided[x + y * newSize] = f / 4.0 + e / 2.0 + c / 4.0;
        }
    }

    _subdivided = move(newSubdivided);
    _nsubdivisions++;
}

