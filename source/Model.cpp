#include "Model.h"
#include "BlobReader.h"
#include "BSP.h"
#include "Core.h"
#include "ResourceCache.h"
#include "Texture.h"
#include "TextureLookup5.h"
#include "TextureLookup8.h"

static vector<TriangleFan> unpackTriangleFans(BlobReader& reader)
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
    BlobReader reader(data, size);

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
        unpackTriangleFans(reader);
        skipBSP(reader, 1);
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
        skipBSP(reader, 0);
    }

    if(flags & 0x8)
    {
        reader.read<uint32_t>();
    }

    reader.assertEnd();
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

bool Model::needsDepthSort() const
{
    return _needsDepthSort;
}

unique_ptr<Destructable>& Model::renderData()
{
    return _renderData;
}
