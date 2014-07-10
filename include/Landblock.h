#ifndef BZR_LANDBLOCK_H
#define BZR_LANDBLOCK_H

#include "IDestructable.h"
#include "Noncopyable.h"
#include "math/Vec2.h"

class Landblock : Noncopyable
{
public:
    static const int GRID_SIZE = 9;
    static const double SQUARE_SIZE;
    static const double LANDBLOCK_SIZE;

    Landblock(const void* data, size_t length);

    const double* getSubdividedData() const;
    size_t getSubdividedSize() const;

    double getOriginalHeight(Vec2 point) const;
    
    double getSubdividedHeight(Vec2 point) const;

    unique_ptr<IDestructable>& renderData();

private:
    PACK(struct RawData
    {
        uint32_t fileid;
        uint32_t flags;
        uint16_t styles[GRID_SIZE][GRID_SIZE];
        uint8_t heights[GRID_SIZE][GRID_SIZE];
        uint8_t pad;
    });

    void subdivide(int n);
    void subdivideOnce();

    RawData _data;

    unique_ptr<double[]> _original;

    unique_ptr<double[]> _subdivided;
    int _nsubdivisions;

    unique_ptr<IDestructable> _renderData;
};

#endif
