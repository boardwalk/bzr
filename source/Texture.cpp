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

Texture::Texture(uint32_t id, const void* data, size_t size) : ResourceImpl{id}
{
    BinReader reader(data, size);

    uint32_t resourceId = reader.readInt();
    assert(resourceId == id);

    uint32_t unk1 = reader.readInt();
    assert(unk1 <= 0xA);

    uint32_t width = reader.readInt();
    assert(width <= 4096);

    uint32_t height = reader.readInt();
    assert(height <= 4096);

    PixelFormat::Value format = static_cast<PixelFormat::Value>(reader.readInt());

    if(format == PixelFormat::kJPEG)
    {
        throw runtime_error("JPEG textures not supported");
    }

    uint32_t pixelsSize = reader.readInt();
    assert(pixelsSize * 8 == width * height * PixelFormat::bitsPerPixel(format));

    const uint8_t* pixels = reader.readRaw(pixelsSize);

    if(PixelFormat::isPaletted(format))
    {
        uint32_t paletteId = reader.readInt();
        palette = Core::get().resourceCache().get(paletteId);
    }

    reader.assertEnd();

    image.init(format, width, height, pixels);

    if(palette)
    {
        image.applyPalette(palette->cast<Palette>());
    }
}

Texture::Texture(uint32_t bgra) : ResourceImpl(ResourceType::kTexture | 0xFFFF)
{
    image.init(PixelFormat::kBGRA32, 1, 1, &bgra);
}
