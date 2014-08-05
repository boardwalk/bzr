#ifndef BZR_TEXTURELOOKUP5_H
#define BZR_TEXTURELOOKUP5_H

#include "Destructable.h"
#include "Resource.h"
#include <vector>

class TextureLookup5 : public ResourceImpl<Resource::TextureLookup5>
{
public:
    TextureLookup5(const void* data, size_t size);

    const vector<ResourcePtr>& textures();

private:
    vector<ResourcePtr> _textures;

};

#endif
