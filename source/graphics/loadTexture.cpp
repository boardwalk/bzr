#include "graphics/loadTexture.h"
#include "BlobReader.h"
#include "Core.h"
#include "DatFile.h"

PACK(struct TextureHeader
{
   uint32_t fileId;
   uint32_t unk;
   uint32_t width;
   uint32_t height;
   uint32_t type;
   uint32_t dataSize;
});

static vector<uint8_t> decodeDXT1(const void* data, int width, int height)
{
    throw runtime_error("DXT1 decoding not yet implemented");
}

static vector<uint8_t> decodeDXT5(const void* data, int width, int height)
{
    throw runtime_error("DXT5 decoding not yet implemented");
}

Texture loadTexture(uint32_t fileId)
{
    auto blob = Core::get().highresDat().read(fileId);

    if(blob.empty())
    {
        blob = Core::get().portalDat().read(fileId);
    }

    if(blob.empty())
    {
        throw runtime_error("Texture not found");
    }

    BlobReader reader(blob.data(), blob.size());

    auto header = reader.readPointer<TextureHeader>();
 
    Texture::Format format;
    int bpp;

    if(header->type == 0x14) // BGR24
    {
        format = Texture::BGR24;
        bpp = 24;
    }
    else if(header->type == 0x15) // BGRA32
    {
        format = Texture::BGRA32;
        bpp = 32;
    }
    else if(header->type == 0x31545844) // DXT1
    {
        format = Texture::BGR24;
        bpp = 4;
    }
    else if(header->type == 0x35545844) // DXT5
    {
        format = Texture::BGRA32;
        bpp = 8;
    }
    else if(header->type == 0x65) // 16-bit paletted
    {
        throw runtime_error("Paletted texture not yet implemented");
    }
    else if(header->type == 0xf3) // RGB24
    {
        format = Texture::RGB24;
        bpp = 24;
    }
    else if(header->type == 0xf4) // A8
    {
        format = Texture::A8;
        bpp = 8;
    }
    else
    {
        throw runtime_error("Unrecognized texture type");
    }

    if(header->width * header->height * bpp / 8 != header->dataSize)
    {
        throw runtime_error("Texture dataSize mismatch");
    }

    auto data = reader.readPointer<uint8_t>(header->dataSize);

    reader.assertEnd();

    vector<uint8_t> decodedData;

    if(header->type == 0x31545844)
    {
        decodedData = decodeDXT1(data, header->width, header->height);
        data = decodedData.data();
    }
    else if(header->type == 0x35545844)
    {
        decodedData = decodeDXT5(data, header->width, header->height);
        data = decodedData.data();
    }

    Texture texture;
    texture.create(format, data, header->width, header->height);
    return move(texture);
}
