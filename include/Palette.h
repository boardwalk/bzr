#ifndef BZR_PALETTE_H
#define BZR_PALETTE_H

#include "Resource.h"
#include <vector>

class Palette : public ResourceImpl<ResourceType::Palette>
{
public:
    struct Color
    {
        uint8_t red;
        uint8_t green;
        uint8_t blue;
        uint8_t alpha;
    };

    Palette(uint32_t id, const void* data, size_t size);

    const vector<Color>& colors() const;

private:
    vector<Color> _colors;
};

#endif