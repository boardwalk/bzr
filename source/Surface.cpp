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
#include "Surface.h"
#include "BinReader.h"
#include "Core.h"
#include "ImgTex.h"
#include "ResourceCache.h"
#include "Texture.h"

enum SurfaceType
{
    kBase1Solid   = 0x00000001,
    kBase1Image   = 0x00000002,
    kBase1Clipmap = 0x00000004,
    kTranslucent  = 0x00000010,
    kDiffuse      = 0x00000020,
    kLuminous     = 0x00000040,
    kAlpha        = 0x00000100,
    kInvAlpha     = 0x00000200,
    kAdditive     = 0x00010000,
    kDetail       = 0x00020000,
    kGouraud      = 0x10000000,
    kStippled     = 0x40000000,
    kPerspective  = 0x80000000
};

Surface::Surface(uint32_t id, const void* data, size_t size) : ResourceImpl{id}
{
    BinReader reader(data, size);

    uint32_t flags = reader.readInt();

    if(flags & kBase1Solid)
    {
        uint32_t bgra = reader.readInt();
        ResourcePtr texture(new Texture(bgra));
        imgTex.reset(new ImgTex(texture));
    }
    else if(flags & (kBase1Image | kBase1Clipmap))
    {
        uint32_t textureId = reader.readInt();
        imgTex = Core::get().resourceCache().get(textureId);
        assert(imgTex->resourceType() == ResourceType::kImgTex);

        uint32_t paletteId = reader.readInt();
        assert(paletteId == 0);
    }
    else
    {
        assert(false);
    }

    translucency = reader.readFloat();
    luminosity = reader.readFloat();
    diffuse = reader.readFloat();

    reader.assertEnd();
}

Surface::Surface(ResourcePtr imgTex) : ResourceImpl(ResourceType::kSurface | 0xFFFF), imgTex(imgTex)
{
    assert(imgTex->resourceType() == ResourceType::kImgTex);
}
