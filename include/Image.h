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
#ifndef BZR_GRAPHICS_IMAGE_H
#define BZR_GRAPHICS_IMAGE_H

class Palette;

struct ImageFormat
{
    enum Value
    {
        kInvalid = 0x00,
        kBGR24 = 0x14,
        kBGRA32 = 0x15,
        kA16 = 0x17,
        kA16_2 = 0x1A,
        kA8_2 = 0x1C,
        kPaletted8 = 0x29,
        kPaletted16 = 0x65,
        kRGB24 = 0xF3,
        kA8 = 0xF4,
        kJPEG = 0x1F4,
        kDXT1 = 0x31545844,
        kDXT3 = 0x33545844,
        kDXT5 = 0x35545844
    };

    static int bitsPerPixel(Value format);
    static bool isPaletted(Value format);
    static bool isCompressed(Value format);
    static bool hasAlpha(Value format);
};

class Image
{
public:
    Image();

    void init(ImageFormat::Value newFormat, int newWidth, int newHeight, const void* newData);

    void applyPalette(const Palette& palette);
    void scale(int newWidth, int newHeight);
    void fill(int value);
    void flipVertical();

    ImageFormat::Value format() const;
    int width() const;
    int height() const;
    size_t size() const;
    const uint8_t* data() const;
    bool hasAlpha() const;

private:
    template<class T>
    void applyPalette(const Palette& palette);

    void updateHasAlpha();

    ImageFormat::Value format_;
    int width_;
    int height_;
    vector<uint8_t> data_;
    bool hasAlpha_;
};

#endif
