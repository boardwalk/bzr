#include "Vertex.h"
#include "BlobReader.h"

void Vertex::read(BlobReader& reader)
{
	auto numTexCoords = reader.read<uint16_t>();
    texCoords.resize(numTexCoords);

    position.x = reader.read<float>();
    position.y = reader.read<float>();
    position.z = reader.read<float>();

    normal.x = reader.read<float>();
    normal.y = reader.read<float>();
    normal.z = reader.read<float>();

    for(auto& texCoord : texCoords)
    {
        texCoord.x = reader.read<float>();
        texCoord.y = reader.read<float>();
    }
}
