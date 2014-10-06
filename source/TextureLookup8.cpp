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
#include "TextureLookup8.h"
#include "BinReader.h"
#include "Core.h"
#include "ResourceCache.h"
#include "Texture.h"
#include "TextureLookup5.h"

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

TextureLookup8::TextureLookup8(uint32_t id, const void* data, size_t size) : ResourceImpl{id}
{
    BinReader reader(data, size);

    uint32_t flags = reader.readInt();

    if(flags & kBase1Solid)
    {
        uint32_t bgra = reader.readInt();
        ResourcePtr texture(new Texture(bgra));
        textureLookup5.reset(new TextureLookup5(texture));
    }
    else if(flags & (kBase1Image | kBase1Clipmap))
    {
        uint32_t textureId = reader.readInt();
        textureLookup5 = Core::get().resourceCache().get(textureId);
        assert(textureLookup5->resourceType() == ResourceType::kTextureLookup5);

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

TextureLookup8::TextureLookup8(ResourcePtr textureLookup5) : ResourceImpl(ResourceType::kTextureLookup8 | 0xFFFF), textureLookup5(textureLookup5)
{
    assert(textureLookup5->resourceType() == ResourceType::kTextureLookup5);
}
