#ifndef BZR_LANDBLOCK_H
#define BZR_LANDBLOCK_H

#include "math/Vec2.h"
#include "Destructable.h"
#include "Noncopyable.h"

class Landblock : Noncopyable
{
public:
    static const int GRID_SIZE = 9;
    static const double SQUARE_SIZE;
    static const double LANDBLOCK_SIZE;

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

    const double* getOriginalData() const;
    size_t getOriginalSize() const;

    const double* getSubdividedData() const;
    size_t getSubdividedSize() const;

    double getOriginalHeight(Vec2 point) const;
    
    double getSubdividedHeight(Vec2 point) const;

    int getStyle(Vec2 point) const;

    unique_ptr<Destructable>& renderData();

    bool splitNESW(int x, int y) const;

private:
    void subdivide(int n);
    void subdivideOnce();

    RawData _data;

    unique_ptr<double[]> _original;

    unique_ptr<double[]> _subdivided;
    int _nsubdivisions;

    unique_ptr<Destructable> _renderData;
};

#endif
