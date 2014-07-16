#ifndef BZR_LANDBLOCK_H
#define BZR_LANDBLOCK_H

#include "Destructable.h"
#include "Noncopyable.h"
#include <vector>

class Landblock : Noncopyable
{
public:
    static const int GRID_SIZE = 9;
    static const double SQUARE_SIZE;
    static const double LANDBLOCK_SIZE;
    static const int OFFSET_MAP_SIZE = 512;

    PACK(struct RawData
    {
        uint32_t fileId;
        uint32_t flags;
        uint16_t styles[GRID_SIZE][GRID_SIZE];
        uint8_t heights[GRID_SIZE][GRID_SIZE];
        uint8_t pad;
    });

    Landblock(const void* data, size_t length);

    const RawData& getRawData() const;

    double getHeight(double x, double y) const;

    const uint16_t* getOffsetMap() const;
    int getOffsetMapSize() const;
    double getOffsetMapBase() const;
    double getOffsetMapScale() const;

    bool splitNESW(int x, int y) const;

    unique_ptr<Destructable>& renderData();

private:
    void buildOffsetMap();

    RawData _data;

    vector<uint16_t> _offsetMap;
    double _offsetMapBase;
    double _offsetMapScale;

    unique_ptr<Destructable> _renderData;
};

#endif
