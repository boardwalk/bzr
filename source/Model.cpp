#include "Model.h"
#include "BlobReader.h"
#include "Core.h"
#include "ResourceCache.h"
#include "Texture.h"
#include "TextureLookup5.h"
#include "TextureLookup8.h"

static void skipBSP(BlobReader&, int);

static void skipBSPLeaf(BlobReader& reader, int treeType)
{
    reader.read<uint32_t>(); // leaf index

    if(treeType != 1)
    {
        return;
    }

    reader.read<uint32_t>(); // if 1, sphere parameters are valid and there are indices

    reader.read<float>(); // sx
    reader.read<float>(); // sy
    reader.read<float>(); // sz
    reader.read<float>(); // sr

    auto indexCount = reader.read<uint32_t>();

    for(auto i = 0u; i < indexCount; i++)
    {
        reader.read<uint16_t>();
    }
}

static void skipBSPPortal(BlobReader& reader, int treeType)
{
    reader.read<float>(); // px
    reader.read<float>(); // py
    reader.read<float>(); // pz
    reader.read<float>(); // pd

    skipBSP(reader, treeType);
    skipBSP(reader, treeType);

    if(treeType != 0)
    {
        return;
    }

    reader.read<float>(); // sx
    reader.read<float>(); // sy
    reader.read<float>(); // sz
    reader.read<float>(); // sr

    auto triCount = reader.read<uint32_t>();
    auto polyCount = reader.read<uint32_t>();

    for(auto i = 0u; i < triCount; i++)
    {
        reader.read<uint16_t>(); // wTriIndex
    }

    for(auto i = 0u; i < polyCount; i++)
    {
        reader.read<uint16_t>(); // wIndex
        reader.read<uint16_t>(); // wWhat
    }
}

static void skipBSPNode(BlobReader& reader, int treeType, uint32_t nodeType)
{
    reader.read<float>(); // px
    reader.read<float>(); // py
    reader.read<float>(); // pz
    reader.read<float>(); // pd


   if(nodeType == 0x42506e6e || nodeType == 0x4250496e) // BPnn, BPIn
   {
        skipBSP(reader, treeType);
   }
   else if(nodeType == 0x4270494e || nodeType == 0x42706e4e) // BpIN, BpnN
   {
        skipBSP(reader, treeType);
    }
    else if(nodeType == 0x4250494e || nodeType == 0x42506e4e) // BPIN, BPnN
    {
        skipBSP(reader, treeType);
        skipBSP(reader, treeType);
    }

    if(treeType == 0 || treeType == 1)
    {
        reader.read<float>(); // sx
        reader.read<float>(); // sy
        reader.read<float>(); // sz
        reader.read<float>(); // sr
    }

    if(treeType != 0)
    {
        return;
    }

    auto indexCount = reader.read<uint32_t>();

    for(auto i = 0u; i < indexCount; i++)
    {
        reader.read<uint16_t>();
    }
}

static void skipBSP(BlobReader& reader, int treeType)
{
    auto nodeType = reader.read<uint32_t>();

    if(nodeType == 0x4c454146) // LEAF
    {
        skipBSPLeaf(reader, treeType);
    }
    else if(nodeType == 0x504f5254) // PORT
    {
        skipBSPPortal(reader, treeType);
    }
    else
    {
        skipBSPNode(reader, treeType, nodeType);
    }
}

vector<Model::Primitive> unpackPrimitives(BlobReader& reader)
{
    auto numPrimitives = reader.readVarInt();

    vector<Model::Primitive> primitives(numPrimitives);

    for(auto pi = 0; pi < numPrimitives; pi++)
    {
        auto& primitive = primitives[pi];

        auto primitiveNum = reader.read<uint16_t>();
        assert(primitiveNum == pi);

        auto numIndices = reader.read<uint8_t>();
        primitive.indices.resize(numIndices);

        auto primFlags = reader.read<uint8_t>();
        assert(primFlags == 0x0 || primFlags == 0x1 || primFlags == 0x4);

        auto primFlags2 = reader.read<uint32_t>();
        assert(primFlags2 == 0x0 || primFlags2 == 0x1 || primFlags2 == 0x2);

        primitive.texIndex = reader.read<uint16_t>();

        reader.read<uint16_t>();

        for(auto& index : primitive.indices)
        {
            index.vertexIndex = reader.read<uint16_t>();
        }

        if(primFlags != 0x04)
        {
            for(auto& index : primitive.indices)
            {
                index.texCoordIndex = reader.read<uint8_t>();
            }
        }
        else
        {
            // This is some sort of lighting/partitioning poly, don't render it
            primitive.indices.clear();
        }

        if(primFlags2 == 0x02)
        {
            for(auto pvi = 0; pvi < numIndices; pvi++)
            {
                reader.read<uint8_t>();
            }
        }
    }

    return primitives;
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

        auto format = texture->cast<TextureLookup8>().textureLookup5().texture().image().format();
        _needsDepthSort = _needsDepthSort || ImageFormat::hasAlpha(format);
    }

    auto one = reader.read<uint32_t>();
    assert(one == 1);

    auto numVertices = reader.read<uint16_t>();
    _vertices.resize(numVertices);

    auto flags2 = reader.read<uint16_t>();
    assert(flags2 == 0x0000 || flags2 == 0x8000);

    for(auto vi = 0; vi < numVertices; vi++)
    {
        auto& vertex = _vertices[vi];

        auto vertexNum = reader.read<uint16_t>();
        assert(vertexNum == vi);

        auto numTexCoords = reader.read<uint16_t>();
        vertex.texCoords.resize(numTexCoords);

        vertex.position.x = reader.read<float>();
        vertex.position.y = reader.read<float>();
        vertex.position.z = reader.read<float>();

        vertex.normal.x = reader.read<float>();
        vertex.normal.y = reader.read<float>();
        vertex.normal.z = reader.read<float>();

        for(auto& texCoord : vertex.texCoords)
        {
            texCoord.x = reader.read<float>();
            texCoord.y = reader.read<float>();
        }
    }

    if(flags == 0x2 || flags == 0xA)
    {
        reader.read<float>();
        reader.read<float>();
        reader.read<float>();
    }

    if(flags & 0x1)
    {
        unpackPrimitives(reader);
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
        _primitives = unpackPrimitives(reader);
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

const vector<Model::Vertex>& Model::vertices() const
{
    return _vertices;
}

const vector<Model::Primitive>& Model::primitives() const
{
    return _primitives;
}

bool Model::needsDepthSort() const
{
    return _needsDepthSort;
}

unique_ptr<Destructable>& Model::renderData()
{
    return _renderData;
}
