#ifndef BZR_TEXTURELOOKUP_H
#define BZR_TEXTURELOOKUP_H

#include "Destructable.h"
#include <vector>

class TextureLookup : public Destructable
{
public:
    TextureLookup(const void* data, size_t size);

    const vector<uint32_t>& textures();

private:
    vector<uint32_t> _textures;

};

#endif
