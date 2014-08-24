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

    auto input = _data.data();
    auto inputEnd = _data.data() + _data.size();

    if(_format == ImageFormat::BGRA32)
    {
        while(input < inputEnd)
        {
            if(input[3] != 0xFF)
            {
                _hasAlpha = true;
                return;
            }

            input += 4;
        }
    }
    else if(_format == ImageFormat::DXT1)
    {
        while(input < inputEnd)
        {
            auto c0 = *(const uint16_t*)input;
            auto c1 = *(const uint16_t*)(input + 2);
            auto ctab = *(const uint32_t*)(input + 4);

            if(c0 <= c1)
            {
                while(ctab)
                {
                    if((ctab & 0x3) == 0x3)
                    {
                        _hasAlpha = true;
                        return;
                    }

                    ctab >>= 2;
                }
            }

            input += 8;
        }
    }
    else if(_format == ImageFormat::A8 || _format == ImageFormat::DXT3 || _format == ImageFormat::DXT5)
    {
        // There's no reason to use these formats unless you have alpha
        // So let's just assume it's they do
        _hasAlpha = true;
    }
}
