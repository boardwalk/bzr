#include "graphics/Image.h"
#include "BlobReader.h"
#include "Core.h"
#include "DatFile.h"
#include <algorithm>

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
    (void)data;
    (void)width;
    (void)height;

    throw runtime_error("DXT1 decoding not yet implemented");
}

static vector<uint8_t> decodeDXT5(const void* data, int width, int height)
{
    (void)data;
    (void)width;
    (void)height;

    throw runtime_error("DXT5 decoding not yet implemented");
}

Image::Image() : _format(Invalid)
{}

Image::Image(Image&& other)
{
    _format = other._format;
    _data = move(other._data);
}

Image& Image::operator=(Image&& other)
{
    _format = other._format;
    _data = move(other._data);
    return *this;
}

void Image::create(Format format, int width, int height)
{
    _format = format;
    _width = width;
    _height = height;
    _data.clear();
    _data.resize(_width * _height * numChannels());
}

void Image::load(uint32_t fileId)
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
 
    int bpp;

    if(header->type == 0x14) // BGR24
    {
        _format = BGR24;
        bpp = 24;
    }
    else if(header->type == 0x15) // BGRA32
    {
        _format = BGRA32;
        bpp = 32;
    }
    else if(header->type == 0x31545844) // DXT1
    {
        _format = BGR24;
        bpp = 4;
    }
    else if(header->type == 0x35545844) // DXT5
    {
        _format = BGRA32;
        bpp = 8;
    }
    else if(header->type == 0x65) // 16-bit paletted
    {
        throw runtime_error("Paletted texture not yet implemented");
    }
    else if(header->type == 0xf3) // RGB24
    {
        _format = RGB24;
        bpp = 24;
    }
    else if(header->type == 0xf4) // A8
    {
        _format = A8;
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

    if(header->type == 0x31545844)
    {
        _data = decodeDXT1(data, header->width, header->height);
    }
    else if(header->type == 0x35545844)
    {
        _data = decodeDXT5(data, header->width, header->height);
    }
    else
    {
        _data.assign(data, data + header->dataSize);
    }

    _width = header->width;
    _height = header->height;

    // OpenGL expects the first row to be the bottom row
    flipVertical();
}

void Image::blit(const Image& image, int x, int y)
{
    if(_format != image._format)
    {
        throw runtime_error("Image format mismatch in blit");
    }

    if(x < 0 || x + image._width > _width || y < 0 || y + image._height > _height)
    {
        throw runtime_error("Image destination out of range");  
    }

    auto nchannels = numChannels();

    for(int row = 0; row < image._height; row++)
    {
        memcpy(_data.data() + (x + (y + row) * _width) * nchannels,
               image._data.data() + (row * image._width) * nchannels,
               image._width * nchannels);
    }
}

void Image::scale(int newWidth, int newHeight)
{
    if(newWidth == _width && newHeight == _height)
    {
        return;
    }
    
    int nchannels = numChannels();

    vector<uint8_t> newData(newWidth * newHeight * nchannels);

    for(auto dstY = 0; dstY < newHeight; dstY++)
    {
        for(auto dstX = 0; dstX < newWidth; dstX++)
        {
            auto srcFX = double(dstX) / double(newWidth) * double(_width);
            auto srcFY = double(dstY) / double(newHeight) * double(_height);

            auto srcX = (int)srcFX;
            auto srcY = (int)srcFY;

            auto xDiff = srcFX - srcX;
            auto yDiff = srcFY - srcY;

            auto xOpposite = 1.0 - xDiff;
            auto yOpposite = 1.0 - yDiff;

#define SRCPX(x, y, cn) (double)_data[(min(x, _width - 1) + min(y, _height - 1) * _width) * nchannels + cn]
#define DSTPX(x, y, cn) newData[((x) + (y) * newWidth) * nchannels + cn]

            for(auto c = 0; c < nchannels; c++)
            {
                DSTPX(dstX, dstY, c) = uint8_t(
                    (SRCPX(srcX, srcY, c) * xOpposite + SRCPX(srcX + 1, srcY, c) * xDiff) * yOpposite +
                    (SRCPX(srcX, srcY + 1, c) * xOpposite + SRCPX(srcX + 1, srcY + 1, c) * xDiff) * yDiff);
            }

#undef SRCPX
#undef DSTPX
        }
    }

    _data = move(newData);
    _width = newWidth;
    _height = newHeight;
}

void Image::fill(int value)
{
    memset(_data.data(), value, _data.size());
}

void Image::flipVertical()
{
    auto stride = _width * numChannels();

    vector<uint8_t> rowBuf(stride);

    for(auto y = 0; y < _height / 2; y++)
    {
        memcpy(rowBuf.data(), _data.data() + y * stride, stride);
        memcpy(_data.data() + stride * y, _data.data() + (_height - y - 1) * stride, stride);
        memcpy(_data.data() + (_height - y - 1) * stride, rowBuf.data(), stride);
    }
}

Image::Format Image::format() const
{
    return _format;
}

int Image::numChannels() const
{
    switch(_format)
    {
        case A8:
            return 1;
        case BGR24:
        case RGB24:
            return 3;
        case BGRA32:
            return 4;
        default:
            assert(!"Invalid Image::Format");
            return -1;
    }
}

const void* Image::data() const
{
    return _data.data();
}

int Image::width() const
{
    return _width;
}

int Image::height() const
{
    return _height;
}
