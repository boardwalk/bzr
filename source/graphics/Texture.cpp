#include "graphics/Texture.h"
#include "BlobReader.h"

PACK(struct TextureHeader
{
   uint32_t fileId;
   uint32_t unk;
   uint32_t width;
   uint32_t height;
   uint32_t type;
   uint32_t dataSize;
});

Texture::Texture() : _handle(0)
{}

Texture::Texture(TextureFormat format, const void* data, int width, int height) : _handle(0)
{
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

void Texture::bind()
{
    if(_handle != 0)
    {
        throw runtime_error("Attempt to bind empty texture");
    }

    glBindTexture(GL_TEXTURE_2D, _handle);
}

void Texture::destroy()
{
   if(_handle != 0)
   {
      glDeleteTextures(1, &_handle);
      _handle = 0;
   }
}

Texture Texture::fromBlob(const void* blob, size_t size)
{
    BlobReader reader(blob, size);

    auto header = reader.read<TextureHeader>();
 
    TextureFormat format;
    uint32_t bytesPerPixel;

    if(header->type == 0x14) // BGR24
    {
        format = BGR24;
        bytesPerPixel = 3;
    }
    else if(header->type == 0x15) // BGRA32
    {
        format = BGRA32;
        bytesPerPixel = 4;
    }
    else if(header->type == 0x31545844) // DXT1
    {
        throw runtime_error("DXT1 textures not yet implemented");
    }
    else if(header->type == 0x35545844) // DXT5
    {
        throw runtime_error("DXT5 textures not yet implemented");
    }
    else if(header->type == 0x65) // 16-bit paletted
    {
        throw runtime_error("Paletted texture not yet implemented");
    }
    else if(header->type == 0xf3) // RGB24
    {
        format = RGB24;
        bytesPerPixel = 3;
    }
    else if(header->type == 0xf4) // A8
    {
        format = A8;
        bytesPerPixel = 1;
    }
    else
    {
        throw runtime_error("Unrecognized texture type");
    }

    if(header->width * header->height * bytesPerPixel != header->dataSize)
    {
        throw runtime_error("Texture dataSize mismatch");
    }

    auto data = reader.read<uint8_t>(header->dataSize);

    reader.assertEnd();

    return Texture(format, data, header->width, header->height);
}

