#include "Model.h"
#include "BlobReader.h"

Model::Model(const void* data, size_t size)
{
    BlobReader reader(data, size);

    auto fileId = reader.read<uint32_t>();
    assert((fileId & 0xFF000000) == 0x01000000);

    auto flags = reader.read<uint32_t>();
    assert(flags == 0x2 || flags == 0x3 || flags == 0xA || flags == 0xB);

    auto numTextures = reader.read<uint8_t>();
    _textures.resize(numTextures);

    for(auto ti = 0; ti < numTextures; ti++)
    {
        _textures[ti] = reader.read<uint32_t>();
    }

    auto one = reader.read<uint32_t>();
    assert(one == 1);

    auto numVertices = reader.read<uint16_t>();
    _vertices.resize(numVertices);

    auto flags2 = reader.read<uint16_t>();
    assert(flags2 == 0x0000 || flags == 0x8000);

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

        for(auto tci = 0; tci < numTexCoords; tci++)
        {
            auto& texCoord = vertex.texCoords[tci];

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

    auto numPrimitives = reader.readVarInt();
    _primitives.resize(numPrimitives);

    for(auto pi = 0; pi < numPrimitives; pi++)
    {
        auto& primitive = _primitives[pi];

        auto primitiveNum = reader.read<uint16_t>();
        assert(primitiveNum == pi);

        auto numIndices = reader.read<uint8_t>();
        primitive.vertexIndices.resize(numIndices);
        primitive.texCoordIndices.resize(numIndices);

        auto primFlags = reader.read<uint8_t>();
        assert(primFlags == 0x0 || primFlags == 0x1 || primFlags == 0x4);

        auto primFlags2 = reader.read<uint8_t>();
        assert(primFlags2 == 0x0 || primFlags2 == 0x1 || primFlags2 == 0x2);

        reader.read<uint8_t>();
        reader.read<uint8_t>();
        reader.read<uint8_t>();
        reader.read<uint8_t>();
        reader.read<uint8_t>();
        reader.read<uint8_t>();
        reader.read<uint8_t>();

        for(auto pvi = 0; pvi < numIndices; pvi++)
        {
            primitive.vertexIndices[pvi] = reader.read<uint16_t>();
            assert(primitive.vertexIndices[pvi] < numVertices);
        }

        if(primFlags != 0x04)
        {
            for(auto pvi = 0; pvi < numIndices; pvi++)
            {
                primitive.texCoordIndices[pvi] = reader.read<uint8_t>();
                assert(primitive.texCoordIndices[pvi] < _vertices[primitive.vertexIndices[pvi]].texCoords.size());
            }
        }

        if(primFlags2 == 0x02)
        {
            for(auto pvi = 0; pvi < numIndices; pvi++)
            {
                reader.read<uint8_t>();
            }
        }
    }
}

const vector<uint32_t>& Model::textures() const
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

unique_ptr<Destructable>& Model::renderData()
{
    return _renderData;
}
