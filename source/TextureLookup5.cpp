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
#include "TextureLookup5.h"
#include "BinReader.h"
#include "Core.h"
#include "ResourceCache.h"
#include "Texture.h"

TextureLookup5::TextureLookup5(uint32_t id,  const void* data, size_t size) : ResourceImpl(id)
{
    BinReader reader(data, size);

    auto resourceId = reader.read<uint32_t>();
    assert(resourceId == id);
    assert((resourceId & 0xFF000000) == 0x05000000);

    auto zero = reader.read<uint32_t>();
    assert(zero == 0);

    auto two = reader.read<uint8_t>();
    assert(two == 2);

    auto numTextures = reader.read<uint32_t>();
    assert(numTextures > 0);

    // This seems to be a list of textures by decreasing quality, as the first ones in the list are in highres.dat
    // We're just going to pick the first and roll with it

    auto textureId = reader.read<uint32_t>();
    _texture = Core::get().resourceCache().get(textureId);
    assert(_texture->resourceType() == ResourceType::Texture);

    for(auto i = 1u; i < numTextures; i++)
    {
        reader.read<uint32_t>();
    }

    reader.assertEnd();
}

TextureLookup5::TextureLookup5(ResourcePtr texture) : ResourceImpl(0), _texture(texture)
{
    assert(_texture->resourceType() == ResourceType::Texture);
}

const Texture& TextureLookup5::texture() const
{
    return _texture->cast<Texture>();
}