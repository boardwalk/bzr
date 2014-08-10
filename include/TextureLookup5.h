#ifndef BZR_TEXTURELOOKUP5_H
#define BZR_TEXTURELOOKUP5_H

#include "Destructable.h"
#include "Resource.h"
#include <vector>

class TextureLookup5 : public ResourceImpl<Resource::TextureLookup5>
{
public:
    TextureLookup5(uint32_t id, const void* data, size_t size);

    const ResourcePtr& texture();

private:
    ResourcePtr _texture;

};

#endif
