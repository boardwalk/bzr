#ifndef BZR_TEXTURE_H
#define BZR_TEXTURE_H

#include "Image.h"
#include "Resource.h"

class Texture : public ResourceImpl<ResourceType::Texture>
{
public:
    Texture(uint32_t id, const void* data, size_t size);
    explicit Texture(uint32_t bgra);

    const Image& image() const;
    const ResourcePtr& palette() const;

private:
    Image _image;
    ResourcePtr _palette;
};

#endif
