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

    auto header = reader.read<TextureHeader>();
 
    Texture::Format format;
    uint32_t bytesPerPixel;

    if(header->type == 0x14) // BGR24
    {
        format = Texture::BGR24;
        bytesPerPixel = 3;
    }
    else if(header->type == 0x15) // BGRA32
    {
        format = Texture::BGRA32;
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
        format = Texture::RGB24;
        bytesPerPixel = 3;
    }
    else if(header->type == 0xf4) // A8
    {
        format = Texture::A8;
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

