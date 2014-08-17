#include "TextureLookup8.h"
#include "BlobReader.h"
#include "Core.h"
#include "ResourceCache.h"
#include "Texture.h"
#include "TextureLookup5.h"

TextureLookup8::TextureLookup8(uint32_t id, const void* data, size_t size) : ResourceImpl(id)
{
    BlobReader reader(data, size);

    auto flags = reader.read<uint8_t>();
    assert(flags == 0x01 || flags == 0x02 || flags == 0x04 || flags == 0x11 || flags == 0x12 || flags == 0x14);

    reader.read<uint8_t>();
    reader.read<uint8_t>();
    reader.read<uint8_t>();

    if(flags & 0x01)
    {
        uint32_t bgra = reader.read<uint32_t>();
        ResourcePtr texture(new Texture(bgra));
        ResourcePtr textureLookup5(new TextureLookup5(texture));
        _textureLookup5 = textureLookup5;
    }
    else
    {
        auto textureId = reader.read<uint32_t>();
        _textureLookup5 = Core::get().resourceCache().get(textureId);
        assert(_textureLookup5->resourceType() == ResourceType::TextureLookup5);

        auto zero = reader.read<uint32_t>();
        assert(zero == 0);
    }

    // I suspect these may be texture coordinates within the texture
    auto f1 = reader.read<float>();

    if(flags & 0x10)
    {
        assert(f1 != 0.0);
    }
    else
    {
        assert(f1 == 0.0);
    }

    reader.read<float>();
    reader.read<float>();

    reader.assertEnd();
}

const TextureLookup5& TextureLookup8::textureLookup5() const
{
    return _textureLookup5->cast<TextureLookup5>();
}
