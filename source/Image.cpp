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

int PixelFormat::bitsPerPixel(Value format)
{
    switch(format)
    {
        case kBGR24:
            return 24;
        case kBGRA32:
            return 32;
        case kA16:
            return 16;
        case kA16_2:
            return 16;
        case kA8_2:
            return 8;
        case kPaletted8:
            return 8;
        case kPaletted16:
            return 16;
        case kRGB24:
            return 24;
        case kA8:
            return 8;
        case kDXT1:
            return 4;
        case kDXT3:
            return 8;
        case kDXT5:
            return 8;
        default:
            break;
    }

    throw runtime_error("Invalid format");
}

bool PixelFormat::isPaletted(Value format)
{
    return format == kPaletted8 || format == kPaletted16;
}

bool PixelFormat::isCompressed(Value format)
{
    return format == kDXT1 || format == kDXT3 || format == kDXT5;
}

bool PixelFormat::hasAlpha(Value format)
{
    return format == kBGRA32 || format == kPaletted8 || format == kPaletted16 || format == kA8 || format == kDXT3 || format == kDXT5;
}

Image::Image() : format_(PixelFormat::kInvalid), width_(0), height_(0), hasAlpha_(false)
{}

void Image::init(PixelFormat::Value newFormat, int newWidth, int newHeight, const void* newData)
{
    format_ = newFormat;
    width_ = newWidth;
    height_ = newHeight;

    if(newData == nullptr)
    {
        data_.clear();
        data_.resize(width_ * height_ * PixelFormat::bitsPerPixel(format_) / 8);
    }
    else
    {
        data_.assign((const uint8_t*)newData, (const uint8_t*)newData + width_ * height_ * PixelFormat::bitsPerPixel(format_) / 8);
    }

    updateHasAlpha();
}

template<class T>
void Image::applyPalette(const Palette& palette)
{
    vector<uint8_t> newData(width_ * height_ * 4);

    const T* input = reinterpret_cast<const T*>(data_.data());
    const T* inputEnd = input + width_ * height_;

    uint8_t* output = newData.data();

    while(input < inputEnd)
    {
        T paletteIndex = *input & (palette.colors.size() - 1);
        Palette::Color color = palette.colors[paletteIndex];

        *output++ = color.blue;
        *output++ = color.green;
        *output++ = color.red;
        *output++ = color.alpha;

        input++;
    }

    data_ = move(newData);
    format_ = PixelFormat::kBGRA32;
    updateHasAlpha();
}

void Image::applyPalette(const Palette& palette)
{
    if(format_ == PixelFormat::kPaletted8)
    {
        applyPalette<uint8_t>(palette);

    }
    else if(format_ == PixelFormat::kPaletted16)
    {
        applyPalette<uint16_t>(palette);
    }
    else
    {
        throw runtime_error("Cannot apply palette to this format");
    }
}

void Image::scale(int newWidth, int newHeight)
{
    if(newWidth == width_ && newHeight == height_)
    {
        return;
    }

    if(PixelFormat::isCompressed(format_))
    {
        throw runtime_error("Cannot scale compressed image");
    }

    int nchannels = PixelFormat::bitsPerPixel(format_) / 8;

    vector<uint8_t> newData(newWidth * newHeight * nchannels);

    for(int dstY = 0; dstY < newHeight; dstY++)
    {
        for(int dstX = 0; dstX < newWidth; dstX++)
        {
            fp_t srcFX = static_cast<fp_t>(dstX) / static_cast<fp_t>(newWidth) * static_cast<fp_t>(width_);
            fp_t srcFY = static_cast<fp_t>(dstY) / static_cast<fp_t>(newHeight) * static_cast<fp_t>(height_);

            int srcX = static_cast<int>(srcFX);
            int srcY = static_cast<int>(srcFY);

            fp_t xDiff = srcFX - srcX;
            fp_t yDiff = srcFY - srcY;

            fp_t xOpposite = fp_t(1.0) - xDiff;
            fp_t yOpposite = fp_t(1.0) - yDiff;

#define SRCPX(x, y, cn) static_cast<fp_t>(data_[(min(x, width_ - 1) + min(y, height_ - 1) * width_) * nchannels + cn])
#define DSTPX(x, y, cn) newData[((x) + (y) * newWidth) * nchannels + cn]

            for(int c = 0; c < nchannels; c++)
            {
                DSTPX(dstX, dstY, c) = static_cast<uint8_t>(
                    (SRCPX(srcX, srcY, c) * xOpposite + SRCPX(srcX + 1, srcY, c) * xDiff) * yOpposite +
                    (SRCPX(srcX, srcY + 1, c) * xOpposite + SRCPX(srcX + 1, srcY + 1, c) * xDiff) * yDiff);
            }

#undef SRCPX
#undef DSTPX
        }
    }

    data_ = move(newData);
    width_ = newWidth;
    height_ = newHeight;
}

void Image::flipVertical()
{
    if(PixelFormat::isCompressed(format_))
    {
        throw runtime_error("Cannot flip compressed image");
    }

    int stride = width_ * PixelFormat::bitsPerPixel(format_) / 8;

    vector<uint8_t> rowBuf(stride);

    for(int y = 0; y < height_ / 2; y++)
    {
        memcpy(rowBuf.data(), data_.data() + y * stride, stride);
        memcpy(data_.data() + stride * y, data_.data() + (height_ - y - 1) * stride, stride);
        memcpy(data_.data() + (height_ - y - 1) * stride, rowBuf.data(), stride);
    }
}

void Image::fill(int value)
{
    memset(data_.data(), value, data_.size());
    updateHasAlpha();
}

PixelFormat::Value Image::format() const
{
    return format_;
}

int Image::width() const
{
    return width_;
}

int Image::height() const
{
    return height_;
}

size_t Image::size() const
{
    return data_.size();
}

const uint8_t* Image::data() const
{
    return data_.data();
}

bool Image::hasAlpha() const
{
    return hasAlpha_;
}

void Image::updateHasAlpha()
{
    hasAlpha_ = false;

    uint8_t* input = data_.data();
    uint8_t* inputEnd = data_.data() + data_.size();

    if(format_ == PixelFormat::kBGRA32)
    {
        while(input < inputEnd)
        {
            if(input[3] != 0xFF)
            {
                hasAlpha_ = true;
                return;
            }

            input += 4;
        }
    }
    else if(format_ == PixelFormat::kDXT1)
    {
        while(input < inputEnd)
        {
            uint16_t c0 = *reinterpret_cast<const uint16_t*>(input);
            uint16_t c1 = *reinterpret_cast<const uint16_t*>(input + 2);
            uint32_t ctab = *reinterpret_cast<const uint32_t*>(input + 4);

            if(c0 <= c1)
            {
                while(ctab)
                {
                    if((ctab & 0x3) == 0x3)
                    {
                        hasAlpha_ = true;
                        return;
                    }

                    ctab >>= 2;
                }
            }

            input += 8;
        }
    }
    else if(format_ == PixelFormat::kA8 || format_ == PixelFormat::kDXT3 || format_ == PixelFormat::kDXT5)
    {
        // There's no reason to use these formats unless you have alpha
        // So let's just assume it's they do
        hasAlpha_ = true;
    }
}
