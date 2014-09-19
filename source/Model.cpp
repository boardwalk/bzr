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
#include "Model.h"
#include "BinReader.h"
#include "BSP.h"
#include "Core.h"
#include "ResourceCache.h"
#include "Texture.h"
#include "TextureLookup5.h"
#include "TextureLookup8.h"

static vector<TriangleFan> readTriangleFans(BinReader& reader)
{
    uint16_t numTriangleFans = reader.readVarInt();

    vector<TriangleFan> triangleFans(numTriangleFans);

    for(uint16_t i = 0; i < numTriangleFans; i++)
    {
        uint16_t triangleFanNum = reader.read<uint16_t>();
        assert(triangleFanNum == i);

        triangleFans[i].read(reader);
    }

    return triangleFans;
}

Model::Model(uint32_t id, const void* data, size_t size) : ResourceImpl{id}, needsDepthSort{false}
{
    BinReader reader(data, size);

    uint32_t resourceId = reader.read<uint32_t>();
    assert(resourceId == id);

    uint32_t flags = reader.read<uint32_t>();
    assert(flags == 0x2 || flags == 0x3 || flags == 0xA || flags == 0xB);

    uint8_t numTextures = reader.read<uint8_t>();
    textures.resize(numTextures);

    for(ResourcePtr& texture : textures)
    {
        uint32_t textureId = reader.read<uint32_t>();
        texture = Core::get().resourceCache().get(textureId);

        bool hasAlpha = texture->cast<TextureLookup8>()
            .textureLookup5->cast<TextureLookup5>()
            .texture->cast<Texture>()
            .image.hasAlpha();
        needsDepthSort = needsDepthSort || hasAlpha;
    }

    uint32_t one = reader.read<uint32_t>();
    assert(one == 1);

    uint16_t numVertices = reader.read<uint16_t>();
    vertices.resize(numVertices);

    uint16_t flags2 = reader.read<uint16_t>();
    assert(flags2 == 0x0000 || flags2 == 0x8000);

    for(uint16_t i = 0; i < numVertices; i++)
    {
        uint16_t vertexNum = reader.read<uint16_t>();
        assert(vertexNum == i);

        vertices[i].read(reader);
    }

    if(flags == 0x2 || flags == 0xA)
    {
        reader.read<float>();
        reader.read<float>();
        reader.read<float>();
    }

    if(flags & 0x1)
    {
        hitTriangleFans = readTriangleFans(reader);
        hitTree = readBSP(reader, 1);
    }

    if(flags == 0x3 || flags == 0xB)
    {
       reader.read<float>();
       reader.read<float>();
       reader.read<float>();
    }

    if(flags & 0x2)
    {
        triangleFans = readTriangleFans(reader);
        readBSP(reader, 0);
    }

    if(flags & 0x8)
    {
        // Seems to be a reference to an 0x11 file? No idea what these are!
        reader.read<uint32_t>();
    }

    reader.assertEnd();
}

Model::~Model()
{}
