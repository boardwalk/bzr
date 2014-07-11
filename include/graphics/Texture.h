#ifndef BZR_GRAPHICS_TEXTURE_H
#define BZR_GRAPHICS_TEXTURE_H

#include "Noncopyable.h"

class Texture : Noncopyable
{
public:
    enum TextureFormat
    {
        BGR24, BGRA32, RGB24, A8
    };

    static Texture fromBlob(const void* blob, size_t size);

    Texture();
    Texture(TextureFormat format, const void* data, int width, int height);
    Texture(Texture&& other);
    ~Texture();
    Texture& operator=(Texture&& other);

    void bind();
    void destroy();

private:
    GLuint _handle;
};

#endif
