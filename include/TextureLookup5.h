#ifndef BZR_TEXTURELOOKUP5_H
#define BZR_TEXTURELOOKUP5_H

#include "Destructable.h"
#include "Resource.h"
#include <vector>

class Texture;

class TextureLookup5 : public ResourceImpl<ResourceType::TextureLookup5>
{
public:
    TextureLookup5(uint32_t id, const void* data, size_t size);
    explicit TextureLookup5(ResourcePtr texture);

    const Texture& texture() const;

private:
    ResourcePtr _texture;
};

#endif
