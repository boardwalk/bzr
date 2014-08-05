#ifndef BZR_TEXTURELOOKUP8_H
#define BZR_TEXTURELOOKUP8_H

#include "Resource.h"

class TextureLookup8 : public ResourceImpl<Resource::TextureLookup8>
{
public:
    TextureLookup8(const void* data, size_t size);

    const ResourcePtr& texture() const;

private:
    ResourcePtr _texture;
};

#endif