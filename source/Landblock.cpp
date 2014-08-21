#include "Landblock.h"
#include "BlobReader.h"
#include "Core.h"
#include "DatFile.h"
#include "LandblockManager.h"
#include <algorithm>

const double Landblock::SQUARE_SIZE = 24.0;
const double Landblock::LANDBLOCK_SIZE = (Landblock::GRID_SIZE - 1) * Landblock::SQUARE_SIZE;

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

Landblock::Landblock(const void* data, size_t length)
{
    if(length != sizeof(RawData))
    {
        throw runtime_error("Bad landblock data length");
    }

    memcpy(&_rawData, data, sizeof(_rawData));
}

Landblock::Landblock(Landblock&& other)
{
    _rawData = other._rawData;
    _objects = move(other._objects);
    _offsetMap = move(other._offsetMap);
    _offsetMapBase = other._offsetMapBase;
    _offsetMapScale = other._offsetMapScale;
    _normalMap = move(other._normalMap);
    _renderData = move(other._renderData);
}

void Landblock::init()
{
    if(!_offsetMap.empty())
    {
        return;
    }

    if(_rawData.flags)
    {
        initObjects();
    }

    // sample another two times at the edges so we don't have to clamp our bicubic resample
    static const int sampleSize = GRID_SIZE;
    static const int edgeSize = 2;
    static const int totalSampleSize = sampleSize + edgeSize * 2;

    vector<double> sample(totalSampleSize * totalSampleSize);

    for(auto sy = 0; sy < totalSampleSize; sy++)
    {
        for(auto sx = 0; sx < totalSampleSize; sx++)
        {
            auto lx = double(sx - edgeSize) / double(sampleSize - 1) * LANDBLOCK_SIZE;
            auto ly = double(sy - edgeSize) / double(sampleSize - 1) * LANDBLOCK_SIZE;
            sample[sx + sy * totalSampleSize] = calcHeightUnbounded(lx, ly);
        }
    }

    vector<double> resample(OFFSET_MAP_SIZE * OFFSET_MAP_SIZE);

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
                    p[px][py] = sample[(edgeSize + ix + px - 1) + (edgeSize + iy + py - 1) * totalSampleSize];
                }
            }

            auto lx = double(ox) / double(OFFSET_MAP_SIZE - 1) * LANDBLOCK_SIZE;
            auto ly = double(oy) / double(OFFSET_MAP_SIZE - 1) * LANDBLOCK_SIZE;

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
                double offset = resample[ox + oy * OFFSET_MAP_SIZE];
                _offsetMap[ox + oy * OFFSET_MAP_SIZE] = (uint16_t)((offset - _offsetMapBase) / _offsetMapScale * double(0xFFFF));
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

            auto lx1 = double(ox1) / double(OFFSET_MAP_SIZE - 1) * LANDBLOCK_SIZE;
            auto lx2 = double(ox2) / double(OFFSET_MAP_SIZE - 1) * LANDBLOCK_SIZE;
            auto ly1 = double(oy1) / double(OFFSET_MAP_SIZE - 1) * LANDBLOCK_SIZE;
            auto ly2 = double(oy2) / double(OFFSET_MAP_SIZE - 1) * LANDBLOCK_SIZE;

            double h1 = resample[ox1 + oy1 * OFFSET_MAP_SIZE] + calcHeight(lx1, ly1);
            double h2 = resample[ox2 + oy1 * OFFSET_MAP_SIZE] + calcHeight(lx2, ly1);
            double h3 = resample[ox1 + oy2 * OFFSET_MAP_SIZE] + calcHeight(lx1, ly2);

            Vec3 a(lx2 - lx1, 0.0, h2 - h1);
            Vec3 b(0.0, ly2 - ly1, h3 - h1);

            auto n = a.cross(b).normalize() * 0.5 + Vec3(0.5, 0.5, 0.5);
            _normalMap[(ox + oy * OFFSET_MAP_SIZE) * 3] = (uint8_t)(n.x * double(0xFF));
            _normalMap[(ox + oy * OFFSET_MAP_SIZE) * 3 + 1] = (uint8_t)(n.y * double(0xFF));
            _normalMap[(ox + oy * OFFSET_MAP_SIZE) * 3 + 2] = (uint8_t)(n.z * double(0xFF));
        }
    }
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

    double h1 = _rawData.heights[ix][iy] * 2.0;
    double h2 = _rawData.heights[min(ix + 1, GRID_SIZE - 1)][iy] * 2.0;
    double h3 = _rawData.heights[ix][min(iy + 1, GRID_SIZE - 1)] * 2.0;
    double h4 = _rawData.heights[min(ix + 1, GRID_SIZE - 1)][min(iy + 1, GRID_SIZE - 1)] * 2.0;

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

double Landblock::calcHeightUnbounded(double x, double y) const
{
    auto thisId = id();

    while(x < 0.0)
    {
        thisId = LandblockId(thisId.x() - 1, thisId.y());
        x += LANDBLOCK_SIZE;
    }

    while(x >= LANDBLOCK_SIZE)
    {
        thisId = LandblockId(thisId.x() + 1, thisId.y());
        x -= LANDBLOCK_SIZE;
    }

    while(y < 0.0)
    {
        thisId = LandblockId(thisId.x(), thisId.y() - 1);
        y += LANDBLOCK_SIZE;
    }

    while(y >= LANDBLOCK_SIZE)
    {
        thisId = LandblockId(thisId.x(), thisId.y() + 1);
        y -= LANDBLOCK_SIZE;
    }

    auto it = Core::get().landblockManager().find(thisId);

    if(it == Core::get().landblockManager().end())
    {
        throw logic_error("landblock not found");
    }
    
    return it->second.calcHeight(x, y);
}

LandblockId Landblock::id() const
{
    uint8_t x = uint8_t((_rawData.fileId >> 24) & 0xFF);
    uint8_t y = uint8_t((_rawData.fileId >> 16) & 0xFF);
    return LandblockId(x, y);
}

const Landblock::RawData& Landblock::rawData() const
{
    return _rawData;
}

const vector<Object>& Landblock::objects() const
{
    return _objects;
}

const vector<Structure>& Landblock::structures() const
{
    return _structures;
}

const uint16_t* Landblock::offsetMap() const
{
    return _offsetMap.data();
}

double Landblock::offsetMapBase() const
{
    return _offsetMapBase;
}

double Landblock::offsetMapScale() const
{
    return _offsetMapScale;
}

const uint8_t* Landblock::normalMap() const
{
    return _normalMap.data();
}

bool Landblock::isSplitNESW(int x, int y) const
{
    // credits to Akilla
    uint32_t tx = ((_rawData.fileId >> 24) & 0xFF) * 8 + x;
    uint32_t ty = ((_rawData.fileId >> 16) & 0xFF) * 8 + y;
    uint32_t v = tx * ty * 0x0CCAC033 - tx * 0x421BE3BD + ty * 0x6C1AC587 - 0x519B8F25;
    return (v & 0x80000000) != 0;
}

unique_ptr<Destructable>& Landblock::renderData()
{
    return _renderData;
}

void Landblock::initObjects()
{
    auto baseFileId = _rawData.fileId & 0xFFFF0000;

    auto blob = Core::get().cellDat().read(baseFileId | 0xFFFE);

    BlobReader reader(blob.data(), blob.size());

    auto fid = reader.read<uint32_t>();
    assert(fid == _rawData.fileId - 1);

    auto numStructures = reader.read<uint32_t>();

    for(auto si = 0u; si < numStructures; si++)
    {
        auto structBlob = Core::get().cellDat().read(baseFileId | (0x0100 + si));

        if(structBlob.empty())
        {
            throw runtime_error("Structure not found");
        }

        _structures.emplace_back(structBlob.data(), structBlob.size());
    }

    auto numObjects = reader.read<uint16_t>();
    _objects.resize(numObjects);

    auto unk1 = reader.read<uint16_t>();
    assert(unk1 == 0);

    for(auto oi = 0u; oi < numObjects; oi++)
    {
        _objects[oi].read(reader);
    }

    auto numObjectsEx = reader.read<uint16_t>();
    _objects.resize(numObjects + numObjectsEx);

    auto unk2 = reader.read<uint16_t>();
    // I don't know what this is, but it means there's more data
    assert(unk2 == 0 || unk2 == 1);

    for(auto oi = 0u; oi < numObjectsEx; oi++)
    {
        _objects[numObjects + oi].read(reader);

        reader.read<uint32_t>();
        auto numChunks = reader.read<uint32_t>();

        // credits to Akilla
        for(auto ci = 0u; ci < numChunks; ci++)
        {
            reader.read<uint32_t>();
            reader.read<uint16_t>();
            auto numSubChunks = reader.read<uint16_t>();

            for(auto sci = 0; sci < numSubChunks; sci++)
            {
                reader.read<uint16_t>();
            }

            if(numSubChunks & 1)
            {
                reader.read<uint16_t>();
            }
        }
    }
}
