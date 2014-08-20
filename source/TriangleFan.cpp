#include "TriangleFan.h"
#include "BlobReader.h"

void TriangleFan::read(BlobReader& reader)
{
    auto numIndices = reader.read<uint8_t>();
    indices.resize(numIndices);

    auto flags = reader.read<uint8_t>();
    assert(flags == 0x0 || flags == 0x1 || flags == 0x4);

    auto flags2 = reader.read<uint32_t>();
    assert(flags2 == 0x0 || flags2 == 0x1 || flags2 == 0x2);

    texIndex = reader.read<uint16_t>();

    reader.read<uint16_t>();

    for(auto& index : indices)
    {
        index.vertexIndex = reader.read<uint16_t>();
    }

    if(flags != 0x04)
    {
        for(auto& index : indices)
        {
            index.texCoordIndex = reader.read<uint8_t>();
        }
    }
    else
    {
        // This is some sort of lighting/partitioning poly, don't render it
        indices.clear();
    }

    if(flags2 == 0x02)
    {
        for(auto pvi = 0; pvi < numIndices; pvi++)
        {
            reader.read<uint8_t>();
        }
    }
}
