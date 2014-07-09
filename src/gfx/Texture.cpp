#include "gfx/Texture.h"

Texture::Texture(const void* data, int width, int height, TextureFormat format)
{
    GLint glformat;
    GLint gltype;

    switch(format)
    {
        case RGB16:
            glformat = GL_RGB;
            gltype = GL_UNSIGNED_SHORT_5_6_5;
            break;
        case RGB24:
            glformat = GL_RGB;
            gltype = GL_UNSIGNED_BYTE;
            break;
        case RGBA32:
            glformat = GL_RGBA;
            gltype = GL_UNSIGNED_BYTE;
            break;
        case A8:
            glformat = GL_ALPHA;
            gltype = GL_UNSIGNED_BYTE;
            break;
        default:
            assert(!"Invalid TextureFormat");
            break;
    }

    glGenTextures(1, &_handle);
    glBindTexture(GL_TEXTURE_2D, _handle);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, glformat, width, height, 0, glformat, gltype, data);
}

Texture::~Texture()
{
    glDeleteTextures(1, &_handle);
}

void Texture::bind()
{
    glBindTexture(GL_TEXTURE_2D, _handle);
}

