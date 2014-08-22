#include "Palette.h"
#include "BlobReader.h"

Palette::Palette(uint32_t id, const void* data, size_t size) : ResourceImpl(id)
{
    BlobReader reader(data, size);

    auto resourceId = reader.read<uint32_t>();
    assert(resourceId == id);
    assert((resourceId & 0xFF000000) == 0x04000000);

    auto numColors = reader.read<uint32_t>();
    assert(numColors == 2048);
    _colors.resize(numColors);

    for(auto& color : _colors)
    {
        color.blue = reader.read<uint8_t>();
        color.green = reader.read<uint8_t>();
        color.red = reader.read<uint8_t>();
        color.alpha = reader.read<uint8_t>();
    }

    reader.assertEnd();
}

const vector<Palette::Color>& Palette::colors() const
{
    return _colors;
}