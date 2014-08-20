#ifndef BZR_GRAPHICS_TEXTUREATLAS_H
#define BZR_GRAPHICS_TEXTUREATLAS_H

#include "Noncopyable.h"
#include "Resource.h"
#include <unordered_map>

class TextureAtlas : Noncopyable
{
public:
    struct TextureInfo
    {
        ResourcePtr resource;
        uint32_t index;
    };

    TextureAtlas();
    ~TextureAtlas();

    int get(uint32_t resourceId);
    void bind();

private:
    void generate();

    GLuint _atlas;
    GLuint _atlasToc;
    int _nextIndex;
    int _buildIndex;
    unordered_map<uint32_t, TextureInfo> _textures;
};

#endif