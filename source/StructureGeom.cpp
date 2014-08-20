#include "StructureGeom.h"
#include "BlobReader.h"

StructureGeom::StructureGeom(uint32_t id, const void* data, size_t size) : ResourceImpl(id)
{
    BlobReader reader(data, size);

    auto resourceId = reader.read<uint32_t>();
    assert(resourceId == id);
    assert((resourceId & 0xFF000000) == 0x0D000000);

    reader.read<uint32_t>(); // unk1
    reader.read<uint32_t>(); // unk2

    auto numTriangleFans = reader.read<uint32_t>();
    _triangleFans.resize(numTriangleFans);

    reader.read<uint32_t>(); // unk3
    reader.read<uint32_t>(); // unk4
    reader.read<uint32_t>(); // unk5

    auto numVertices = reader.read<uint16_t>();
    _vertices.resize(numVertices);

    reader.read<uint16_t>(); // unk6

    for(auto i = 0u; i < numVertices; i++)
    {
        auto vertexNum = reader.read<uint16_t>();
        assert(vertexNum == i);

        _vertices[i].read(reader);
    }

    for(auto i = 0u; i < numTriangleFans; i++)
    {
        auto triangleFanNum = reader.read<uint16_t>();
        assert(triangleFanNum == i);

        _triangleFans[i].read(reader);
    }

    reader.read<uint16_t>(); // unk7
    reader.read<uint16_t>(); // unk8
}

const vector<Vertex>& StructureGeom::vertices() const
{
    return _vertices;
}

const vector<TriangleFan>& StructureGeom::triangleFans() const
{
    return _triangleFans;
}