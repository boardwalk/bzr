#include "Texture.h"
#include "BlobReader.h"
#include "Core.h"
#include "ResourceCache.h"

// Converts a 16-bit RGB 5:6:5 to 32-bit BGRA value (with max alpha)
static uint32_t upconvert(uint16_t c)
{
    auto r = (c & 0x1F) * 0xFF / 0x1F;
    auto g = ((c >> 5) & 0x3F) / 0x3F;
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
    vector<uint8_t> result(width * height * 4);

    for(auto by = 0; by < height / 4; by++)
    {
        for(auto bx = 0; bx < width / 4; bx++)
        {
            auto inOffset = (bx + by * width) * 8;

            uint32_t c[4];
            c[0] = upconvert(*(const uint16_t*)(data + inOffset));
            c[1] = upconvert(*(const uint16_t*)(data + inOffset + 2));

            if(c[0] > c[1])
            {
                c[2] = interpolate(c[0], c[1], 2, 1, 3);
                c[3] = interpolate(c[0], c[1], 1, 2, 3);
            }
            else
            {
                c[2] = interpolate(c[0], c[1], 1, 1, 2);
                c[3] = 0;
            }

            auto tab = *(const uint32_t*)(data + inOffset + 4);

            for(auto py = 0; py < 4; py++)
            {
                for(auto px = 0; px < 4; px++)
                {
                    auto outOffset = ((bx + px) + (by + py) * width) * 4;
                    auto cn = (tab >> (px + py * 4)) & 0x3;
                    *((uint32_t*)result.data() + outOffset) = c[cn];
                }
            }
        }
    }

    return result;
}

static vector<uint8_t> decodeDXT5(const uint8_t* data, int width, int height)
{
    vector<uint8_t> result(width * height * 4);

    (void)data;
    (void)width;
    (void)height;

    return result;
}

Texture::Texture(const void* data, size_t size)
{
    BlobReader reader(data, size);

    auto fileId = reader.read<uint32_t>();
    assert((fileId & 0xFF000000) == 0x06000000);

    auto unk1 = reader.read<uint32_t>();
    assert(unk1 <= 0xA);

    _width = reader.read<uint32_t>();
    assert(_width <= 2048);

    _height = reader.read<uint32_t>();
    assert(_height <= 2048);

    _type = (Type)reader.read<uint32_t>();

    int bitsPerPixel;

    switch(_type)
    {
        case BGR24:
            bitsPerPixel = 24;
            break;
        case BGRA32:
            bitsPerPixel = 32;
            break;
        case DXT1:
            bitsPerPixel = 4;
            break;
        case DXT5:
            bitsPerPixel = 8;
            break;
        case Paletted16:
            bitsPerPixel = 16;
            break;
        case RGB24:
            bitsPerPixel = 24;
            break;
        case Alpha8:
            bitsPerPixel = 8;
            break;
        default:
            throw runtime_error("Unknown texture type");
    }

    auto pixelsSize = reader.read<uint32_t>();
    assert(pixelsSize * 8 == _width * _height * bitsPerPixel);

    auto pixels = reader.readPointer<uint8_t>(pixelsSize);
    _pixels.assign(pixels, pixels + pixelsSize);

    if(_type == Paletted16)
    {
        auto paletteId = reader.read<uint32_t>();
        _palette = Core::get().resourceCache().get(paletteId);
    }

    reader.assertEnd();

    if(_type == DXT1)
    {
        assert((_width & 0x3) == 0);
        assert((_height & 0x3) == 0);

        // Convert the 4BPP DXT1 image to a 32BPP BGRA image
        _pixels = decodeDXT1(_pixels.data(), _width, _height);
        _type = BGRA32;
    }
    else if(_type == DXT5)
    {
        assert((_width & 0x3) == 0);
        assert((_height & 0x3) == 0);

        // Convert the 8BPP DXT5 image to a 32BPP BGRA image
        _pixels = decodeDXT5(_pixels.data(), _width, _height);
        _type = BGRA32;
    }
}

uint32_t Texture::width() const
{
    return _width;
}

uint32_t Texture::height() const
{
    return _height;
}

Texture::Type Texture::type() const
{
    return _type;
}

const vector<uint8_t>& Texture::pixels() const
{
    return _pixels;
}

const ResourcePtr& Texture::palette() const
{
    return _palette;
}
