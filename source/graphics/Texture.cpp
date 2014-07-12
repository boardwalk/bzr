#include "graphics/Texture.h"

Texture::Texture() : _handle(0)
{}

Texture::Texture(Texture&& other)
{
    _handle = other._handle;
    other._handle = 0;
}

Texture::~Texture()
{
   destroy();
}

Texture& Texture::operator=(Texture&& other)
{
    destroy();
    _handle = other._handle;
    other._handle = 0;
    return *this;
}

void Texture::create(Format format, const GLvoid* data, int width, int height)
{
    destroy();

    GLint glformat;
    GLint gltype;

    switch(format)
    {
        case BGR24:
            glformat = GL_BGR;
            gltype = GL_UNSIGNED_BYTE;
            break;
        case BGRA32:
            glformat = GL_BGRA;
            gltype = GL_UNSIGNED_BYTE;
            break;
        case RGB24:
            glformat = GL_RGB;
            gltype = GL_UNSIGNED_BYTE;
            break;
        case A8:
            glformat = GL_ALPHA;
            gltype = GL_UNSIGNED_BYTE;
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

void Texture::destroy()
{
    if(_handle != 0)
    {
        glDeleteTextures(1, &_handle);
        _handle = 0;
    }
}

void Texture::bind(int i)
{
    if(_handle == 0)
    {
        throw runtime_error("Attempt to bind empty texture");
    }

    glActiveTexture(GL_TEXTURE0 + i);
    glBindTexture(GL_TEXTURE_2D, _handle);
}
