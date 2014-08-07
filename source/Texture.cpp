#include "Texture.h"
#include "BlobReader.h"
#include "Core.h"
#include "Palette.h"
#include "ResourceCache.h"

Texture::Texture(const void* data, size_t size)
{
    BlobReader reader(data, size);

    auto fileId = reader.read<uint32_t>();
    assert((fileId & 0xFF000000) == 0x06000000);

    auto unk1 = reader.read<uint32_t>();
    assert(unk1 <= 0xA);

    auto width = reader.read<uint32_t>();
    assert(width <= 2048);

    auto height = reader.read<uint32_t>();
    assert(height <= 2048);

    auto format = (Image::Format)reader.read<uint32_t>();

    auto pixelsSize = reader.read<uint32_t>();
    assert(pixelsSize * 8 == width * height * Image::formatBitsPerPixel(format));

    auto pixels = reader.readPointer<uint8_t>(pixelsSize);

    if(format == Image::Paletted16)
    {
        auto paletteId = reader.read<uint32_t>();
        _palette = Core::get().resourceCache().get(paletteId);
    }

    reader.assertEnd();

    _image.init(format, width, height, pixels);
    _image.decompress();

    if(_palette)
    {
        _image.applyPalette(_palette->cast<::Palette>());
    }

    _image.flipVertical();
}

const Image& Texture::image() const
{
    return _image;
}

const ResourcePtr& Texture::palette() const
{
    return _palette;
}
