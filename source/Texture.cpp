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
#include "BinReader.h"
#include "Core.h"
#include "Palette.h"
#include "ResourceCache.h"

Texture::Texture(uint32_t id, const void* data, size_t size) : ResourceImpl(id)
{
    BinReader reader(data, size);

    uint32_t resourceId = reader.read<uint32_t>();
    assert(resourceId == id);

    uint32_t unk1 = reader.read<uint32_t>();
    assert(unk1 <= 0xA);

    uint32_t width = reader.read<uint32_t>();
    assert(width <= 4096);

    uint32_t height = reader.read<uint32_t>();
    assert(height <= 4096);

    ImageFormat::Value format = static_cast<ImageFormat::Value>(reader.read<uint32_t>());

    if(format == ImageFormat::kJPEG)
    {
        throw runtime_error("JPEG textures not supported");
    }

    uint32_t pixelsSize = reader.read<uint32_t>();
    assert(pixelsSize * 8 == width * height * ImageFormat::bitsPerPixel(format));

    const uint8_t* pixels = reader.readPointer<uint8_t>(pixelsSize);

    if(ImageFormat::isPaletted(format))
    {
        uint32_t paletteId = reader.read<uint32_t>();
        palette_ = Core::get().resourceCache().get(paletteId);
    }

    reader.assertEnd();

    image_.init(format, width, height, pixels);

    if(palette_)
    {
        image_.applyPalette(palette_->cast<Palette>());
    }
}

Texture::Texture(uint32_t bgra) : ResourceImpl(ResourceType::kTexture | 0xFFFF)
{
    image_.init(ImageFormat::kBGRA32, 1, 1, &bgra);
}

const Image& Texture::image() const
{
    return image_;
}

const ResourcePtr& Texture::palette() const
{
    return palette_;
}

unique_ptr<Destructable>& Texture::renderData() const
{
    return renderData_;
}
