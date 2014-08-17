#ifndef BZR_TEXTURELOOKUP8_H
#define BZR_TEXTURELOOKUP8_H

#include "Resource.h"

class TextureLookup5;

class TextureLookup8 : public ResourceImpl<Resource::TextureLookup8>
{
public:
    TextureLookup8(uint32_t id, const void* data, size_t size);

    const ::TextureLookup5& textureLookup5() const;

private:
    ResourcePtr _textureLookup5;
};

#endif