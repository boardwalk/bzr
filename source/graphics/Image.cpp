#include "graphics/Image.h"
#include "Palette.h"
#include <algorithm>

// Converts a 16-bit RGB 5:6:5 to 32-bit BGRA value (with max alpha)
static uint32_t upconvert(uint16_t c)
{
    auto r = (c & 0x1F) * 0xFF / 0x1F;
    auto g = ((c >> 5) & 0x3F) * 0xFF / 0x3F;
    auto b = (c >> 11) * 0xFF / 0x1F;

    return b | (g << 8) | (r << 16) | (0xFF << 24);
}

// Interpolates between two 32-bit BGRA values
static uint32_t interpolate(uint32_t c0, uint32_t c1, unsigned int numer0, unsigned int numer1, unsigned int denom)
{
    auto b0 = c0 & 0xFF;
    auto g0 = (c0 >> 8) & 0xFF;
    auto r0 = (c0 >> 16) & 0xFF;
    auto a0 = (c0 >> 24) & 0xFF;

    auto b1 = c1 & 0xFF;
    auto g1 = (c1 >> 8) & 0xFF;
    auto r1 = (c1 >> 16) & 0xFF;
    auto a1 = (c1 >> 24) & 0xFF;

    auto b = (r0 * numer0 + r1 * numer1) / denom;
    auto g = (g0 * numer0 + g1 * numer1) / denom;
    auto r = (b0 * numer0 + b1 * numer1) / denom;
    auto a = (a0 * numer0 + a1 * numer1) / denom;

    return b | (g << 8) | (r << 16) | (a << 24);
}

static vector<uint8_t> decodeDXT1(const uint8_t* data, int width, int height)
{
    assert((width & 0x3) == 0);
    assert((height & 0x3) == 0);

    vector<uint8_t> result(width * height * 4);

    auto input = data;
    auto inputImageEnd = data + width * height / 2;

    auto output = result.data();
    auto outputStride = width * 4;
    auto outputRowEnd = output + outputStride;

    while(input < inputImageEnd)
    {
        auto c0 = *(const uint16_t*)input;
        auto c1 = *(const uint16_t*)(input + 2);

        uint32_t c[4];
        c[0] = upconvert(c0);
        c[1] = upconvert(c1);

        if(c0 > c1)
        {
            c[2] = interpolate(c[0], c[1], 2, 1, 3);
            c[3] = interpolate(c[0], c[1], 1, 2, 3);
        }
        else
        {
            c[2] = interpolate(c[0], c[1], 1, 1, 2);
            c[3] = 0;
        }

        auto tab = *(const uint32_t*)(input + 4);

        for(auto py = 0; py < 4; py++)
        {
            for(auto px = 0; px < 4; px++)
            {
                auto cn = (tab >> (px * 2 + py * 8)) & 0x3;
                *(uint32_t*)(output + px * 4 + py * outputStride) = c[cn];
            }
        }

        input += 8; // 8 bytes per block
        output += 4 * 4; // 4 pixels across per block

        if(output >= outputRowEnd)
        {
            output += 3 * outputStride;
            outputRowEnd = output + outputStride;
        }
    }

    return result;
}

static vector<uint8_t> decodeDXT5(const uint8_t* data, int width, int height)
{
    assert((width & 0x3) == 0);
    assert((height & 0x3) == 0);

    vector<uint8_t> result(width * height * 4);

    (void)data;
    (void)width;
    (void)height;

    // TODO

    return result;
}

Image::Image() : _format(Invalid)
{}

Image::Image(const Image& other)
{
    _data = other._data;
    _width = other._width;
    _height = other._height;
    _format = other._format;
}

Image::Image(Image&& other)
{
    _data = move(other._data);
    _width = other._width;
    _height = other._height;
    _format = other._format;
}

Image& Image::operator=(const Image& other)
{
    _data = other._data;
    _width = other._width;
    _height = other._height;
    _format = other._format;
    return *this;
}

Image& Image::operator=(Image&& other)
{
    _data = move(other._data);
    _width = other._width;
    _height = other._height;
    _format = other._format;
    return *this;
}

void Image::init(Format newFormat, int newWidth, int newHeight, const void* newData)
{
    _format = newFormat;
    _width = newWidth;
    _height = newHeight;

    if(newData == nullptr)
    {
        _data.clear();
        _data.resize(_width * _height * formatBitsPerPixel(_format) / 8);
    }
    else
    {
        _data.assign((const uint8_t*)newData, (const uint8_t*)newData + _width * _height * formatBitsPerPixel(_format) / 8);
    }
}

void Image::decompress()
{
    if(_format == DXT1)
    {
        _data = decodeDXT1(_data.data(), _width, _height);
        _format = BGRA32;
    }
    else if(_format == DXT5)
    {
        _data = decodeDXT5(_data.data(), _width, _height);
        _format = BGRA32;
    }
}

void Image::applyPalette(const Palette& palette)
{
    if(_format != Paletted16)
    {
        return;
    }

    vector<uint8_t> newData(_width * _height * 4);

    auto input = (const uint16_t*)_data.data();
    auto inputEnd = (const uint16_t*)_data.data() + _width * _height;

    auto output = (uint8_t*)newData.data();

    while(input < inputEnd)
    {
        // TODO I have no idea if this is the proper thing to do
        auto paletteIndex = *input & 0x7FF;
        auto color = palette.colors()[paletteIndex];

        *output++ = color.blue;
        *output++ = color.green;
        *output++ = color.red;
        *output++ = color.alpha;

        input++;
    }

    _data = move(newData);
    _format = BGRA32;
}

void Image::scale(int newWidth, int newHeight)
{
    if(newWidth == _width && newHeight == _height)
    {
        return;
    }

    if(_format == DXT1 || _format == DXT5)
    {
        throw runtime_error("Cannot scale compressed image");
    }

    auto nchannels = formatBitsPerPixel(_format) / 8;

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

void Image::flipVertical()
{
    if(_format == DXT1 || _format == DXT5)
    {
        throw runtime_error("Cannot flip compressed image");
    }

    auto stride = _width * formatBitsPerPixel(_format) / 8;

    vector<uint8_t> rowBuf(stride);

    for(auto y = 0; y < _height / 2; y++)
    {
        memcpy(rowBuf.data(), _data.data() + y * stride, stride);
        memcpy(_data.data() + stride * y, _data.data() + (_height - y - 1) * stride, stride);
        memcpy(_data.data() + (_height - y - 1) * stride, rowBuf.data(), stride);
    }
}

void Image::fill(int value)
{
    memset(_data.data(), value, _data.size());
}

Image::Format Image::format() const
{
    return _format;
}

int Image::width() const
{
    return _width;
}

int Image::height() const
{
    return _height;
}

size_t Image::size() const
{
    return _data.size();
}

const void* Image::data() const
{
    return _data.data();
}

int Image::formatBitsPerPixel(Format f)
{
    switch(f)
    {
        case BGR24:
            return 24;
        case BGRA32:
            return 32;
        case RGB24:
            return 24;
        case A8:
            return 8;
        case DXT1:
            return 4;
        case DXT5:
            return 8;
        case Paletted16:
            return 16;
        default:
            break;
    }

    throw runtime_error("Invalid format");
}
