/*
 * Bael'Zharon's Respite
 * Copyright (C) 2014 Daniel Skorupski
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */
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

    if(_palette)
    {
        _image.applyPalette(_palette->cast<Palette>());
    }
}

Texture::Texture(uint32_t bgra) : ResourceImpl(0)
{
    _image.init(ImageFormat::BGRA32, 1, 1, &bgra);
}

const Image& Texture::image() const
{
    return _image;
}

const ResourcePtr& Texture::palette() const
{
    return _palette;
}

unique_ptr<Destructable>& Texture::renderData()
{
    return _renderData;
}
