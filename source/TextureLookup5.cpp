#include "TextureLookup5.h"
#include "BlobReader.h"
#include "Core.h"
#include "ResourceCache.h"

TextureLookup5::TextureLookup5(uint32_t id,  const void* data, size_t size) : ResourceImpl(id)
{
    BlobReader reader(data, size);

    auto fileId = reader.read<uint32_t>();
    assert((fileId & 0xFF000000) == 0x05000000);

    auto zero = reader.read<uint32_t>();
    assert(zero == 0);

    auto two = reader.read<uint8_t>();
    assert(two == 2);

    auto numTextures = reader.read<uint32_t>();
    assert(numTextures > 0);

    // This seems to be a list of textures by decreasing quality, as the first ones in the list are in highres.dat
    // We're just going to pick the first and roll with it

    auto textureId = reader.read<uint32_t>();
    _texture = Core::get().resourceCache().get(textureId);

    for(auto i = 1u; i < numTextures; i++)
    {
        reader.read<uint32_t>();
    }

    reader.assertEnd();
}

const ResourcePtr& TextureLookup5::texture()
{
    return _texture;
}