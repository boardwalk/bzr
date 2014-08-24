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
        Invalid = 0x00,
        BGR24 = 0x14,
        BGRA32 = 0x15,
        RGB24 = 0xF3,
        A8 = 0xF4,
        DXT1 = 0x31545844,
        DXT3 = 0x33545844,
        DXT5 = 0x35545844,
        Paletted16 = 0x65
    };

    static int bitsPerPixel(Value f);
    static bool isCompressed(Value f);
    static bool hasAlpha(Value f);
};

class Image
{
public:
    Image();
    Image(const Image&);
    Image(Image&&);
    Image& operator=(const Image&);
    Image& operator=(Image&&);

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
    void updateHasAlpha();

    ImageFormat::Value _format;
    int _width;
    int _height;
    vector<uint8_t> _data;
    bool _hasAlpha;
};

#endif
