#ifndef BZR_TEXTURE_H
#define BZR_TEXTURE_H

#include "Resource.h"
#include <vector>

class Texture : public ResourceImpl<Resource::Texture>
{
public:
    enum Type
    {
        BGR16 = 0x14,
        BGRA24 = 0x15,
        Paletted16 = 0x65,
        RGB24 = 0xF3,
        Alpha8 = 0xF4,
        DXT1 = 0x31545844,
        DXT5 = 0x35545844
    };

    Texture(const void* data, size_t size);

    uint32_t width() const;
    uint32_t height() const;
    Type type() const;
    const vector<uint8_t>& pixels() const;
    const ResourcePtr& palette() const;

private:
    uint32_t _width;
    uint32_t _height;
    Type _type;
    vector<uint8_t> _pixels;
    ResourcePtr _palette;
};

#endif