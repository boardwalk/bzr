#include "TextureLookup5.h"
#include "BlobReader.h"

TextureLookup5::TextureLookup5(const void* data, size_t size)
{
    BlobReader reader(data, size);

    auto fileId = reader.read<uint32_t>();
    assert((fileId & 0xFF000000) == 0x05000000);

    auto zero = reader.read<uint32_t>();
    assert(zero == 0);

    auto two = reader.read<uint8_t>();
    assert(two == 2);

    auto numTextures = reader.read<uint32_t>();
    _textures.resize(numTextures);

    for(auto i = 0u; i < numTextures; i++)
    {
        _textures[i] = reader.read<uint32_t>();
    }
}

const vector<uint32_t>& TextureLookup5::textures()
{
    return _textures;
}