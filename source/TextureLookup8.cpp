#include "TextureLookup8.h"
#include "BlobReader.h"
#include "Core.h"
#include "ResourceCache.h"

TextureLookup8::TextureLookup8(const void* data, size_t size)
{
    BlobReader reader(data, size);

    auto flags = reader.read<uint32_t>();
    assert(flags == 0x01 || flags == 0x02 || flags == 0x04 || flags == 0x11 || flags == 0x12 || flags == 0x14);

    if(flags & 0x01)
    {
        reader.read<uint32_t>();
    }
    else
    {
        auto textureId = reader.read<uint32_t>();
        _texture = Core::get().resourceCache().get(textureId);

        auto zero = reader.read<uint32_t>();
        assert(zero == 0);
    }

    // I suspect these may be texture coordinates within the texture
    reader.read<float>();
    reader.read<float>();
    reader.read<float>();

    reader.assertEnd();
}
