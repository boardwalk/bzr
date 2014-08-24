/*
 * Bael'Zharon's Respite
 * Copyright (C) 2014 Daniel Skorupski
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */
#include "Image.h"
#include "Palette.h"
#include <algorithm>

// Converts a 16-bit BGR 5:6:5 to 24-bit BGR value
static uint32_t upconvert(uint16_t c)
{
    auto b = (c & 0x1F) * 0xFF / 0x1F;
    auto g = ((c >> 5) & 0x3F) * 0xFF / 0x3F;
    auto r = (c >> 11) * 0xFF / 0x1F;

    return b | (g << 8) | (r << 16);
}

// Interpolates between two 24-bit BGR values
static uint32_t interpolate(uint32_t c0, uint32_t c1, unsigned int numer0, unsigned int numer1, unsigned int denom)
{
    auto b0 = c0 & 0xFF;
    auto g0 = (c0 >> 8) & 0xFF;
    auto r0 = (c0 >> 16) & 0xFF;

    auto b1 = c1 & 0xFF;
    auto g1 = (c1 >> 8) & 0xFF;
    auto r1 = (c1 >> 16) & 0xFF;

    auto b = (b0 * numer0 + b1 * numer1) / denom;
    auto g = (g0 * numer0 + g1 * numer1) / denom;
    auto r = (r0 * numer0 + r1 * numer1) / denom;

    return b | (g << 8) | (r << 16);
}

// Interpolates between two 8-bit alpha values
static uint8_t interpolateAlpha(uint8_t a0, uint8_t a1, unsigned int numer0, unsigned int numer1, unsigned int denom)
{
    return uint8_t((a0 * numer0 + a1 * numer1) / denom);
}

// Converts a 4BPP DXT1-compressed image to a 32BPP BGRA image
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
        auto ctab = *(const uint32_t*)(input + 4);

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

        for(auto py = 0; py < 4; py++)
        {
            for(auto px = 0; px < 4; px++)
            {
                auto cn = (ctab >> (px * 2 + py * 8)) & 0x3;
                auto p = output + px * 4 + py * outputStride;
                *(uint32_t*)p = c[cn] | (0xFF << 24);
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

// Converts an 8BPP DXT3-encoded image to a 32BPP BGRA image
static vector<uint8_t> decodeDXT3(const uint8_t* data, int width, int height)
{
    assert((width & 0x3) == 0);
    assert((height & 0x3) == 0);

    vector<uint8_t> result(width * height * 4);

    auto input = data;
    auto inputImageEnd = data + width * height;

    auto output = result.data();
    auto outputStride = width * 4;
    auto outputRowEnd = output + outputStride;

    while(input < inputImageEnd)
    {
        auto atab = *(const uint64_t*)input;
        auto c0 = *(const uint16_t*)(input + 8);
        auto c1 = *(const uint16_t*)(input + 10);
        auto ctab = *(const uint32_t*)(input + 12);

        uint32_t c[4];
        c[0] = upconvert(c0);
        c[1] = upconvert(c1);
        c[2] = interpolate(c[0], c[1], 2, 1, 3);
        c[3] = interpolate(c[0], c[1], 1, 2, 3);

        for(auto py = 0; py < 4; py++)
        {
            for(auto px = 0; px < 4; px++)
            {
                auto cn = (ctab >> (px * 2 + py * 8)) & 0x3;
                auto a = ((atab >> (px * 4 + py * 16)) & 0xF) * 0xFF / 0xF;
                *(uint32_t*)(output + px * 4 + py * outputStride) = c[cn] | uint32_t(a << 24);
            }
        }

        input += 16; // 16 bytes per block
        output += 4 * 4; // 4 pixels across per block

        if(output >= outputRowEnd)
        {
            output += 3 * outputStride;
            outputRowEnd = output + outputStride;
        }
    }

    return result;
}

// Converts an 8BPP DXT5-encoded image to a 32BPP BGRA image
static vector<uint8_t> decodeDXT5(const uint8_t* data, int width, int height)
{
    assert((width & 0x3) == 0);
    assert((height & 0x3) == 0);

    vector<uint8_t> result(width * height * 4);

    auto input = data;
    auto inputImageEnd = data + width * height;

    auto output = result.data();
    auto outputStride = width * 4;
    auto outputRowEnd = output + outputStride;

    while(input < inputImageEnd)
    {
        auto a0 = *input;
        auto a1 = *(input + 1);
        auto atab = *(const uint64_t*)(input + 2); // we're only using 6 bytes of this, not 8!
        auto c0 = *(const uint16_t*)(input + 8);
        auto c1 = *(const uint16_t*)(input + 10);
        auto ctab = *(const uint32_t*)(input + 12);

        uint8_t a[8];
        a[0] = a0;
        a[1] = a1;

        if(a0 > a1)
        {
            a[2] = interpolateAlpha(a0, a1, 6, 1, 7);
            a[3] = interpolateAlpha(a0, a1, 5, 2, 7);
            a[4] = interpolateAlpha(a0, a1, 4, 3, 7);
            a[5] = interpolateAlpha(a0, a1, 3, 4, 7);
            a[6] = interpolateAlpha(a0, a1, 2, 5, 7);
            a[7] = interpolateAlpha(a0, a1, 1, 6, 7);
        }
        else
        {
            a[2] = interpolateAlpha(a0, a1, 4, 1, 7);
            a[3] = interpolateAlpha(a0, a1, 3, 2, 7);
            a[4] = interpolateAlpha(a0, a1, 2, 3, 7);
            a[5] = interpolateAlpha(a0, a1, 1, 4, 7);
            a[6] = 0x00;
            a[7] = 0xFF;
        }

        uint32_t c[4];
        c[0] = upconvert(c0);
        c[1] = upconvert(c1);
        c[2] = interpolate(c[0], c[1], 2, 1, 3);
        c[3] = interpolate(c[0], c[1], 1, 2, 3);

        for(auto py = 0; py < 4; py++)
        {
            for(auto px = 0; px < 4; px++)
            {
                auto cn = (ctab >> (px * 2 + py * 8)) & 0x3;
                auto an = (atab >> (px * 3 + py * 12)) & 0xF;
                *(uint32_t*)(output + px * 4 + py * outputStride) = c[cn] | (a[an] << 24);
            }
        }

        input += 16; // 16 bytes per block
        output += 4 * 4; // 4 pixels across per block

        if(output >= outputRowEnd)
        {
            output += 3 * outputStride;
            outputRowEnd = output + outputStride;
        }
    }

    return result;
}

int ImageFormat::bitsPerPixel(Value f)
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
        case DXT3:
            return 8;
        case DXT5:
            return 8;
        case Paletted16:
            return 16;
        default:
            break;
    }

    throw runtime_error("Invalid format");
}

bool ImageFormat::isCompressed(Value f)
{
    return f == DXT1 || f == DXT3 || f == DXT5;
}

bool ImageFormat::hasAlpha(Value f)
{
    return f == BGRA32 || f == A8 || f == DXT3 || f == DXT5 || f == Paletted16;
}

Image::Image() : _format(ImageFormat::Invalid)
{}

Image::Image(const Image& other)
{
    _data = other._data;
    _width = other._width;
    _height = other._height;
    _format = other._format;
    _hasAlpha = other._hasAlpha;
}

Image::Image(Image&& other)
{
    _data = move(other._data);
    _width = other._width;
    _height = other._height;
    _format = other._format;
    _hasAlpha = other._hasAlpha;
}

Image& Image::operator=(const Image& other)
{
    _data = other._data;
    _width = other._width;
    _height = other._height;
    _format = other._format;
    _hasAlpha = other._hasAlpha;
    return *this;
}

Image& Image::operator=(Image&& other)
{
    _data = move(other._data);
    _width = other._width;
    _height = other._height;
    _format = other._format;
    _hasAlpha = other._hasAlpha;
    return *this;
}

void Image::init(ImageFormat::Value newFormat, int newWidth, int newHeight, const void* newData)
{
    _format = newFormat;
    _width = newWidth;
    _height = newHeight;

    if(newData == nullptr)
    {
        _data.clear();
        _data.resize(_width * _height * ImageFormat::bitsPerPixel(_format) / 8);
    }
    else
    {
        _data.assign((const uint8_t*)newData, (const uint8_t*)newData + _width * _height * ImageFormat::bitsPerPixel(_format) / 8);
    }

    updateHasAlpha();
}

void Image::decompress()
{
    if(_format == ImageFormat::DXT1)
    {
        _data = decodeDXT1(_data.data(), _width, _height);
        _format = ImageFormat::BGRA32;
        updateHasAlpha();
    }
    else if(_format == ImageFormat::DXT3)
    {
        _data = decodeDXT3(_data.data(), _width, _height);
        _format = ImageFormat::BGRA32;
        updateHasAlpha();
    }
    else if(_format == ImageFormat::DXT5)
    {
        _data = decodeDXT5(_data.data(), _width, _height);
        _format = ImageFormat::BGRA32;
        updateHasAlpha();
    }
}

void Image::applyPalette(const Palette& palette)
{
    if(_format != ImageFormat::Paletted16)
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
    _format = ImageFormat::BGRA32;
    updateHasAlpha();
}

void Image::scale(int newWidth, int newHeight)
{
    if(newWidth == _width && newHeight == _height)
    {
        return;
    }

    if(ImageFormat::isCompressed(_format))
    {
        throw runtime_error("Cannot scale compressed image");
    }

    auto nchannels = ImageFormat::bitsPerPixel(_format) / 8;

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

Image Image::scaleHalf() const
{
    assert(_format == ImageFormat::BGRA32);
    assert(_width != 0 && (_width & 1) == 0);
    assert(_height != 0 && (_height & 1) == 0);

    Image result;
    result._format = ImageFormat::BGRA32;
    result._width = _width / 2;
    result._height = _height / 2;
    result._data.resize(result._width * result._height * 4);
    result._hasAlpha = _hasAlpha;

    static const auto PIXEL_SIZE = 4;

    for(auto y = 0; y < result._height; y++)
    {
        for(auto x = 0; x < result._width; x++)
        {
            for(auto c = 0; c < 4; c++)
            {
                int v0 = _data[(y * 2) * _width * PIXEL_SIZE + (x * 2) * PIXEL_SIZE + c];
                int v1 = _data[(y * 2) * _width * PIXEL_SIZE + (x * 2 + 1) * PIXEL_SIZE + c];
                int v2 = _data[(y * 2 + 1) * _width * PIXEL_SIZE + (x * 2) * PIXEL_SIZE + c];
                int v3 = _data[(y * 2 + 1) * _width * PIXEL_SIZE + (x * 2 + 1) * PIXEL_SIZE + c];
                result._data[y * result._width * PIXEL_SIZE + x * PIXEL_SIZE + c] = uint8_t((v0 + v1 + v2 + v3) / 4);
            }
        }
    }

    return result;
}

void Image::flipVertical()
{
    if(ImageFormat::isCompressed(_format))
    {
        throw runtime_error("Cannot flip compressed image");
    }

    auto stride = _width * ImageFormat::bitsPerPixel(_format) / 8;

    vector<uint8_t> rowBuf(stride);

    for(auto y = 0; y < _height / 2; y++)
    {
        memcpy(rowBuf.data(), _data.data() + y * stride, stride);
        memcpy(_data.data() + stride * y, _data.data() + (_height - y - 1) * stride, stride);
        memcpy(_data.data() + (_height - y - 1) * stride, rowBuf.data(), stride);
    }
}

void Image::blit(const Image& source, int x, int y)
{
    if(&source == this)
    {
        throw runtime_error("Cannot blit image to itself");
    }

    if(x < 0 || y < 0 || x + source.width() > _width || y + source.height() > _height)
    {
        throw runtime_error("Blit destination out of range");
    }

    if(source.format() != _format)
    {
        throw runtime_error("Mismatched format to blit");
    }

    auto inputStride = source.width() * ImageFormat::bitsPerPixel(source.format()) / 8;
    auto input = source.data();
    auto inputEnd = source.data() + source.size();

    auto outputStride = _width * ImageFormat::bitsPerPixel(_format) / 8;    
    auto output = _data.data() + x * ImageFormat::bitsPerPixel(_format) / 8 + y * outputStride;
    
    while(input < inputEnd)
    {
        memcpy(output, input, inputStride);
        
        input += inputStride;
        output += outputStride;
    }
}

void Image::fill(int value)
{
    memset(_data.data(), value, _data.size());
    updateHasAlpha();
}

ImageFormat::Value Image::format() const
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

const uint8_t* Image::data() const
{
    return _data.data();
}

bool Image::hasAlpha() const
{
    return _hasAlpha;
}

void Image::updateHasAlpha()
{
    _hasAlpha = false;

    if(_format == ImageFormat::BGRA32)
    {
        auto input = _data.data() + 3;
        auto inputEnd = _data.data() + _width * _height + 4;

        while(input < inputEnd)
        {
            if(*input != 0xFF)
            {
                _hasAlpha = true;
                return;
            }

            input += 4;
        }
    }
    else if(_format == ImageFormat::A8)
    {
        _hasAlpha = true;
    }

    // Compressed and paletted textures may well have alpha
    // We just don't know at this point
}
