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
    uint16_t numTriangleFans = reader.readPackedShort();

    vector<TriangleFan> triangleFans(numTriangleFans);

    for(uint16_t i = 0; i < numTriangleFans; i++)
    {
        uint16_t triangleFanNum = reader.readShort();
        assert(triangleFanNum == i);

        triangleFans[i].read(reader);
    }

    return triangleFans;
}

Model::Model(uint32_t id, const void* data, size_t size) : ResourceImpl{id}, needsDepthSort{false}
{
    BinReader reader(data, size);

    uint32_t resourceId = reader.readInt();
    assert(resourceId == id);

    uint32_t flags = reader.readInt();
    assert(flags == 0x2 || flags == 0x3 || flags == 0xA || flags == 0xB);

    uint8_t numTextures = reader.readByte();
    textures.resize(numTextures);

    for(ResourcePtr& texture : textures)
    {
        uint32_t textureId = reader.readInt();
        texture = Core::get().resourceCache().get(textureId);

        bool hasAlpha = texture->cast<TextureLookup8>()
            .textureLookup5->cast<TextureLookup5>()
            .texture->cast<Texture>()
            .image.hasAlpha();
        needsDepthSort = needsDepthSort || hasAlpha;
    }

    uint32_t one = reader.readInt();
    assert(one == 1);

    uint16_t numVertices = reader.readShort();
    vertices.resize(numVertices);

    uint16_t flags2 = reader.readShort();
    assert(flags2 == 0x0000 || flags2 == 0x8000);

    for(uint16_t i = 0; i < numVertices; i++)
    {
        uint16_t vertexNum = reader.readShort();
        assert(vertexNum == i);

        vertices[i].read(reader);
    }

    if(flags == 0x2 || flags == 0xA)
    {
        reader.readFloat();
        reader.readFloat();
        reader.readFloat();
    }

    if(flags & 0x1)
    {
        hitTriangleFans = readTriangleFans(reader);
        hitTree = readBSP(reader, 1);
    }

    if(flags == 0x3 || flags == 0xB)
    {
       reader.readFloat();
       reader.readFloat();
       reader.readFloat();
    }

    if(flags & 0x2)
    {
        triangleFans = readTriangleFans(reader);
        readBSP(reader, 0);
    }

    if(flags & 0x8)
    {
        // Seems to be a reference to an 0x11 file? No idea what these are!
        reader.readInt();
    }

    reader.assertEnd();
}

Model::~Model()
{}
