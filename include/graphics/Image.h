#ifndef BZR_GRAPHICS_IMAGE_H
#define BZR_GRAPHICS_IMAGE_H

#include <vector>

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

    void decompress();
    void applyPalette(const Palette& palette);
    void scale(int newWidth, int newHeight);
    void fill(int value);
    void flipVertical();

    ImageFormat::Value format() const;
    int width() const;
    int height() const;
    size_t size() const;
    const void* data() const;
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
