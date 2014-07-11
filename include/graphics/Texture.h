#ifndef BZR_GRAPHICS_TEXTURE_H
#define BZR_GRAPHICS_TEXTURE_H

#include "Noncopyable.h"

class Texture : Noncopyable
{
public:
    enum Format
    {
        BGR24, BGRA32, RGB24, A8
    };

    Texture();
    Texture(Format format, const void* data, int width, int height);
    Texture(Texture&& other);
    ~Texture();
    Texture& operator=(Texture&& other);

    void bind(int i);
    void destroy();

private:
    GLuint _handle;
};

#endif
