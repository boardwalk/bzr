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

static vector<TriangleFan> unpackTriangleFans(BinReader& reader)
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

Model::Model(uint32_t id, const void* data, size_t size) : ResourceImpl(id), _needsDepthSort(false)
{
    BinReader reader(data, size);

    auto resourceId = reader.read<uint32_t>();
    assert(resourceId == id);
    assert((resourceId & 0xFF000000) == 0x01000000);

    auto flags = reader.read<uint32_t>();
    assert(flags == 0x2 || flags == 0x3 || flags == 0xA || flags == 0xB);

    auto numTextures = reader.read<uint8_t>();
    _textures.resize(numTextures);

    for(auto& texture : _textures)
    {
        auto textureId = reader.read<uint32_t>();
        texture = Core::get().resourceCache().get(textureId);

        auto hasAlpha = texture->cast<TextureLookup8>().textureLookup5().texture().image().hasAlpha();
        _needsDepthSort = _needsDepthSort || hasAlpha;
    }

    auto one = reader.read<uint32_t>();
    assert(one == 1);

    auto numVertices = reader.read<uint16_t>();
    _vertices.resize(numVertices);

    auto flags2 = reader.read<uint16_t>();
    assert(flags2 == 0x0000 || flags2 == 0x8000);

    for(auto i = 0u; i < numVertices; i++)
    {
        auto vertexNum = reader.read<uint16_t>();
        assert(vertexNum == i);

        _vertices[i].read(reader);
    }

    if(flags == 0x2 || flags == 0xA)
    {
        reader.read<float>();
        reader.read<float>();
        reader.read<float>();
    }

    if(flags & 0x1)
    {
        _collisionTriangleFans = unpackTriangleFans(reader);
        readBSP(reader, 1);
    }

    if(flags == 0x3 || flags == 0xB)
    {
       reader.read<float>();
       reader.read<float>();
       reader.read<float>();
    }

    if(flags & 0x2)
    {
        _triangleFans = unpackTriangleFans(reader);
        readBSP(reader, 0);
    }

    if(flags & 0x8)
    {
        // Seems to be a reference to an 0x11 file? No idea what these are!
        reader.read<uint32_t>();
    }

    reader.assertEnd();

    // Build bounding box from all vertices
    for(auto& vertex : _vertices)
    {
        _bounds.grow(vertex.position);
    }
}

const vector<ResourcePtr>& Model::textures() const
{
    return _textures;
}

const vector<Vertex>& Model::vertices() const
{
    return _vertices;
}

const vector<TriangleFan>& Model::triangleFans() const
{
    return _triangleFans;
}

const vector<TriangleFan>& Model::collisionTriangleFans() const
{
    return _collisionTriangleFans;
}

const AABB& Model::bounds() const
{
    return _bounds;
}

bool Model::needsDepthSort() const
{
    return _needsDepthSort;
}

unique_ptr<Destructable>& Model::renderData()
{
    return _renderData;
}
