#include "Texture.h"
#include "BlobReader.h"
#include "Core.h"
#include "Palette.h"
#include "ResourceCache.h"

Texture::Texture(uint32_t id, const void* data, size_t size) : ResourceImpl(id)
{
    BlobReader reader(data, size);

    auto resourceId = reader.read<uint32_t>();
    assert(resourceId == id);
    assert((resourceId & 0xFF000000) == 0x06000000);

    auto unk1 = reader.read<uint32_t>();
    assert(unk1 <= 0xA);

    auto width = reader.read<uint32_t>();
    assert(width <= 2048);

    auto height = reader.read<uint32_t>();
    assert(height <= 2048);

    auto format = (ImageFormat::Value)reader.read<uint32_t>();

    auto pixelsSize = reader.read<uint32_t>();
    assert(pixelsSize * 8 == width * height * ImageFormat::bitsPerPixel(format));

    auto pixels = reader.readPointer<uint8_t>(pixelsSize);

    if(format == ImageFormat::Paletted16)
    {
        auto paletteId = reader.read<uint32_t>();
        _palette = Core::get().resourceCache().get(paletteId);
    }

    reader.assertEnd();

    _image.init(format, width, height, pixels);
    _image.decompress();

    if(_palette)
    {
        _image.applyPalette(_palette->cast<Palette>());
    }
}

Texture::Texture(uint32_t bgra) : ResourceImpl(0)
{
    // if we have a solid alpha, create as BGR
    // this will reduce the size of our depth sort list slightly
    auto format = (bgra >> 24 == 0xFF) ? ImageFormat::BGR24 : ImageFormat::BGRA32;
    
    _image.init(format, 1, 1, &bgra);
}

const Image& Texture::image() const
{
    return _image;
}

const ResourcePtr& Texture::palette() const
{
    return _palette;
}
