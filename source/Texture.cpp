#include "Texture.h"
#include "BlobReader.h"
#include "Core.h"
#include "ResourceCache.h"

Texture::Texture(const void* data, size_t size)
{
    BlobReader reader(data, size);

    auto fileId = reader.read<uint32_t>();
    assert((fileId & 0xFF000000) == 0x06000000);

    auto unk1 = reader.read<uint32_t>();
    assert(unk1 <= 0xA);

    _width = reader.read<uint32_t>();
    assert(_width <= 2048);

    _height = reader.read<uint32_t>();
    assert(_height <= 2048);

    _type = (Type)reader.read<uint32_t>();

    int bitsPerPixel;

    switch(_type)
    {
        case BGR16:
            bitsPerPixel = 16;
            break;
        case BGRA24:
            bitsPerPixel = 24;
            break;
        case DXT1:
            bitsPerPixel = 4;
            break;
        case DXT5:
            bitsPerPixel = 8;
            break;
        case Paletted16:
            bitsPerPixel = 16;
            break;
        case RGB24:
            bitsPerPixel = 24;
            break;
        case Alpha8:
            bitsPerPixel = 8;
            break;
        default:
            throw runtime_error("Unknown texture type");
    }

    auto pixelsSize = reader.read<uint32_t>();
    assert(pixelsSize * 8 == _width * _height * bitsPerPixel);

    auto pixels = reader.readPointer<uint8_t>(pixelsSize);
    _pixels.assign(pixels, pixels + pixelsSize);

    if(_type == Paletted16)
    {
        auto paletteId = reader.read<uint32_t>();
        _palette = Core::get().resourceCache().get(paletteId);
    }

    reader.assertEnd();
}

uint32_t Texture::width() const
{
    return _width;
}

uint32_t Texture::height() const
{
    return _height;
}

Texture::Type Texture::type() const
{
    return _type;
}

const vector<uint8_t>& Texture::pixels() const
{
    return _pixels;
}

const ResourcePtr& Texture::palette() const
{
    return _palette;
}
