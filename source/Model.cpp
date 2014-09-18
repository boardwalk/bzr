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
    auto numTriangleFans = reader.readVarInt();

    vector<TriangleFan> triangleFans(numTriangleFans);

    for(auto i = 0u; i < numTriangleFans; i++)
    {
        auto triangleFanNum = reader.read<uint16_t>();
        assert(triangleFanNum == i);

        triangleFans[i].read(reader);
    }

    return triangleFans;
}

Model::Model(uint32_t id, const void* data, size_t size) : ResourceImpl(id), needsDepthSort_(false)
{
    BinReader reader(data, size);

    auto resourceId = reader.read<uint32_t>();
    assert(resourceId == id);

    auto flags = reader.read<uint32_t>();
    assert(flags == 0x2 || flags == 0x3 || flags == 0xA || flags == 0xB);

    auto numTextures = reader.read<uint8_t>();
    textures_.resize(numTextures);

    for(auto& texture : textures_)
    {
        auto textureId = reader.read<uint32_t>();
        texture = Core::get().resourceCache().get(textureId);

        auto hasAlpha = texture->cast<TextureLookup8>().textureLookup5().texture().image().hasAlpha();
        needsDepthSort_ = needsDepthSort_ || hasAlpha;
    }

    auto one = reader.read<uint32_t>();
    assert(one == 1);

    auto numVertices = reader.read<uint16_t>();
    vertices_.resize(numVertices);

    auto flags2 = reader.read<uint16_t>();
    assert(flags2 == 0x0000 || flags2 == 0x8000);

    for(auto i = 0u; i < numVertices; i++)
    {
        auto vertexNum = reader.read<uint16_t>();
        assert(vertexNum == i);

        vertices_[i].read(reader);
    }

    if(flags == 0x2 || flags == 0xA)
    {
        reader.read<float>();
        reader.read<float>();
        reader.read<float>();
    }

    if(flags & 0x1)
    {
        hitTriangleFans_ = readTriangleFans(reader);
        hitTree_ = readBSP(reader, 1);
    }

    if(flags == 0x3 || flags == 0xB)
    {
       reader.read<float>();
       reader.read<float>();
       reader.read<float>();
    }

    if(flags & 0x2)
    {
        triangleFans_ = readTriangleFans(reader);
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

const vector<ResourcePtr>& Model::textures() const
{
    return textures_;
}

const vector<Vertex>& Model::vertices() const
{
    return vertices_;
}

const vector<TriangleFan>& Model::triangleFans() const
{
    return triangleFans_;
}

const vector<TriangleFan>& Model::hitTriangleFans() const
{
    return hitTriangleFans_;
}

const BSPNode* Model::hitTree() const
{
    return hitTree_.get();
}

bool Model::needsDepthSort() const
{
    return needsDepthSort_;
}

unique_ptr<Destructable>& Model::renderData() const
{
    return renderData_;
}
