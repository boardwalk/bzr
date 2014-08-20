#include "StructureGeom.h"
#include "BlobReader.h"
#include "BSP.h"

StructureGeom::StructureGeom(uint32_t id, const void* data, size_t size) : ResourceImpl(id)
{
    BlobReader reader(data, size);

    auto resourceId = reader.read<uint32_t>();
    assert(resourceId == id);
    assert((resourceId & 0xFF000000) == 0x0D000000);

    auto numPieces = reader.read<uint32_t>();
    _pieces.resize(numPieces);

    for(auto pi = 0u; pi < numPieces; pi++)
    {
        auto& piece = _pieces[pi];

        auto pieceNum = reader.read<uint32_t>();
        assert(pieceNum == pi);

        auto numTriangleFans = reader.read<uint32_t>();
        piece.triangleFans.resize(numTriangleFans);

        auto numCollisionTriangleFans = reader.read<uint32_t>();

        auto numShorts = reader.read<uint32_t>();

        auto unk5 = reader.read<uint32_t>();
        assert(unk5 == 1);

        auto numVertices = reader.read<uint32_t>();
        piece.vertices.resize(numVertices);

        for(auto vi = 0u; vi < numVertices; vi++)
        {
            auto vertexNum = reader.read<uint16_t>();
            assert(vertexNum == vi);

            piece.vertices[vi].read(reader);
        }

        for(auto tfi = 0u; tfi < numTriangleFans; tfi++)
        {
            auto triangleFanNum = reader.read<uint16_t>();
            assert(triangleFanNum == tfi);

            piece.triangleFans[tfi].read(reader);
        }

        for(auto si = 0u; si < numShorts; si++)
        {
            reader.read<uint16_t>();
        }
        reader.align();

        skipBSP(reader, 2);

        for(auto ctfi = 0u; ctfi < numCollisionTriangleFans; ctfi++)
        {
            auto triangleFanNum = reader.read<uint16_t>();
            assert(triangleFanNum == ctfi);

            TriangleFan().read(reader);
        }

        skipBSP(reader, 1);

        auto unk7 = reader.read<uint32_t>();
        assert(unk7 == 0 || unk7 == 1);

        if(unk7)
        {
            skipBSP(reader, 0);
        }

        reader.align();
    }

    reader.assertEnd();
}

const vector<StructureGeom::Piece>& StructureGeom::pieces() const
{
    return _pieces;
}
