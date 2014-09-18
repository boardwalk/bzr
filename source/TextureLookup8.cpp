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

TextureLookup8::TextureLookup8(uint32_t id, const void* data, size_t size) : ResourceImpl(id)
{
    BinReader reader(data, size);

    auto flags = reader.read<uint8_t>();
    assert(flags == 0x01 || flags == 0x02 || flags == 0x04 || flags == 0x11 || flags == 0x12 || flags == 0x14);

    reader.read<uint8_t>();
    reader.read<uint8_t>();
    reader.read<uint8_t>();

    if(flags & 0x01)
    {
        uint32_t bgra = reader.read<uint32_t>();
        ResourcePtr texture(new Texture(bgra));
        ResourcePtr textureLookup5(new TextureLookup5(texture));
        textureLookup5_ = textureLookup5;
    }
    else
    {
        auto textureId = reader.read<uint32_t>();
        textureLookup5_ = Core::get().resourceCache().get(textureId);
        assert(textureLookup5_->resourceType() == ResourceType::TextureLookup5);

        auto zero = reader.read<uint32_t>();
        assert(zero == 0);
    }

    // I suspect these may be texture coordinates within the texture
    auto f1 = reader.read<float>();

    if(flags & 0x10)
    {
        assert(f1 != 0.0);
    }
    else
    {
        assert(f1 == 0.0);
    }

    reader.read<float>();
    reader.read<float>();

    reader.assertEnd();
}

TextureLookup8::TextureLookup8(ResourcePtr textureLookup5) : ResourceImpl(ResourceType::TextureLookup8 | 0xFFFF), textureLookup5_(textureLookup5)
{
    assert(textureLookup5_->resourceType() == ResourceType::TextureLookup5);
}

const TextureLookup5& TextureLookup8::textureLookup5() const
{
    return textureLookup5_->cast<TextureLookup5>();
}
