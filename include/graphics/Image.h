#ifndef BZR_GRAPHICS_IMAGE_H
#define BZR_GRAPHICS_IMAGE_H

#include <vector>

class Image
{
public:
    enum Format
    {
        Invalid, BGR24, BGRA32, RGB24, A8
    };

    Image();
    Image(Image&&);
    Image& operator=(Image&&);

    void create(Format format, int width, int height);
    void load(uint32_t fileId);

    void blit(const Image& image, int x, int y);
    void scale(float factor);
    void blur(int windowSize);
    void fill(int value);
    void flipVertical();

    Format format() const;
    int numChannels() const;
    const void* data() const;
    int width() const;
    int height() const;

private:
    Format _format;
    int _width;
    int _height;
    vector<uint8_t> _data;
};

#endif