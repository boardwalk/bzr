#ifndef BZR_LANDBLOCK_H
#define BZR_LANDBLOCK_H

#include "LandblockId.h"
#include "Destructable.h"
#include "Noncopyable.h"
#include <vector>

class Landblock : Noncopyable
{
public:
    static const int GRID_SIZE = 9;
    static const double SQUARE_SIZE;
    static const double LANDBLOCK_SIZE;
    static const int HEIGHT_MAP_SIZE = 64;

    PACK(struct RawData
    {
        uint32_t fileId;
        uint32_t flags;
        uint16_t styles[GRID_SIZE][GRID_SIZE];
        uint8_t heights[GRID_SIZE][GRID_SIZE];
        uint8_t pad;
    });

    Landblock(const void* data, size_t length);
    Landblock(Landblock&& other);

    const RawData& getRawData() const;

    double getHeight(double x, double y) const;

    const uint16_t* getHeightMap() const;
    double getHeightMapBase() const;
    double getHeightMapScale() const;

    bool splitNESW(int x, int y) const;

    unique_ptr<Destructable>& renderData();

private:
    void buildHeightMap();

    RawData _data;

    vector<uint16_t> _heightMap;
    double _heightMapBase;
    double _heightMapScale;

    unique_ptr<Destructable> _renderData;
};

#endif
