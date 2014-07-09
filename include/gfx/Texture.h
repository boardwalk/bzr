#ifndef BZR_TEXTURE_H
#define BZR_TEXTURE_H

#include "Noncopyable.h"

class Texture : Noncopyable
{
public:
    enum TextureFormat
    {
        RGB16, RGB24, RGBA32, A8
    };

    Texture(const void* data, int width, int height, TextureFormat format);
    ~Texture();

private:
    GLuint _handle;
};

#endif
