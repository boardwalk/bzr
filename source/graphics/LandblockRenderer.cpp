#include "graphics/LandblockRenderer.h"
#include "graphics/Image.h"
#include "math/Vec3.h"
#include "Landblock.h"
#include <algorithm>
#include <vector>

// TODO We could only create one indexBuffer per subdivision level
// TODO We could generate the x and y of the vertex data in a shader

static const uint32_t LANDSCAPE_TEXTURES[] =
{
    0x0600379a,
    0x0600379f,
    0x0600382a,
    0x0600382c,
    0x0600382e,
    0x0600383b,
    0x0600383d,
    0x0600383f,
    0x0600384c,
    0x0600388c,
    0x06003794,
    0x06003819,
    0x06003821,
    0x06003824,
    0x06003826,
    0x06003828,
    0x06003830,
    0x06003832,
    0x06003833,
    0x06003835,
    0x06003837,
    0x06003839,
    0x06003843,
    0x06003851,
    0x06003853,
    0x06003856,
    0x06003858,
    0x06003859,
    0x06003891,
    0x06003893,
    0x06006284
};

static const int TERRAIN_ARRAY_SIZE = 256;
static const int TERRAIN_ARRAY_DEPTH = sizeof(LANDSCAPE_TEXTURES) / sizeof(LANDSCAPE_TEXTURES[0]);

static const uint32_t BLEND_TEXTURES[] =
{
    0x06006d61, // vertical, black to white, left of center
    0x06006d6c, // top left corner, black, semi ragged
    0x06006d6d, // top left corner, black, ragged
    0x06006d60, // top left corner, black, rounded
    0x06006d30  // vertical, black to white, very left of center, wavy
};

static const int BLEND_ARRAY_SIZE = 512;
static const int BLEND_ARRAY_DEPTH = sizeof(BLEND_TEXTURES) / sizeof(BLEND_TEXTURES[0]);

LandblockRenderer::LandblockRenderer(const Landblock& landblock)
{
    auto data = landblock.getSubdividedData();
    auto size = (int)landblock.getSubdividedSize();

    vector<float> vertexData;

    for(auto y = 0; y < size; y++)
    {
        for(auto x = 0; x < size; x++)
        {
            auto hc = data[x + y * size];
            auto hw = data[max(x - 1, 0) + y * size];
            auto he = data[min(x + 1, size - 1) + y * size];
            auto hs = data[x + max(y - 1, 0) * size];
            auto hn = data[x + min(y + 1, size - 1) * size];

            double quadSize = Landblock::LANDBLOCK_SIZE / double(size - 1);

            Vec3 ve(quadSize, 0.0, he - hc);
            Vec3 vn(0.0, quadSize, hn - hc);
            Vec3 vw(-quadSize, 0.0, hw - hc);
            Vec3 vs(0.0, -quadSize, hs - hc);

            auto nen = ve.cross(vn);
            auto nnw = vn.cross(vw);
            auto nws = vw.cross(vs);
            auto nse = vs.cross(ve);

            auto n = (nen + nnw + nws + nse).normalize();

            auto lx = double(x) / double(size - 1) * Landblock::LANDBLOCK_SIZE;
            auto ly = double(y) / double(size - 1) * Landblock::LANDBLOCK_SIZE;

            //auto style = landblock.getStyle(Vec2(lx, ly));
            //auto texIndex = (style >> 2) & 0x1F;

            // x, y, z
            vertexData.push_back(lx);
            vertexData.push_back(ly);
            vertexData.push_back(data[x + y * size]);
            // nx, ny, nz
            vertexData.push_back(n.x);
            vertexData.push_back(n.y);
            vertexData.push_back(n.z);
            // tx, ty, tz
            vertexData.push_back(double(x) / double(size - 1) * (Landblock::GRID_SIZE - 1));
            vertexData.push_back(double(y) / double(size - 1) * (Landblock::GRID_SIZE - 1));
            vertexData.push_back(4.0);
        }
    }

    vector<uint16_t> indexData;

    // B-D-F
    // |\|\| ...
    // A-C-E
    // http://en.wikipedia.org/wiki/Triangle_strip
    for(auto y = 0; y < size - 1; y++)
    {
        if(y != 0)
        {
            // end old triangle strip
            indexData.push_back(0xffff);
        }

        auto x = 0;
        
        // begin new triangle strip
        indexData.push_back(x + y * size);
        indexData.push_back(x + (y + 1) * size);

        for(x = 0; x < size - 1; x++)
        {
            indexData.push_back((x + 1) + y * size);
            indexData.push_back((x + 1) + (y + 1) * size);
        }
    }

    assert(indexData.size() < 0xffff);

    _vertexBuffer.create();
    _vertexBuffer.bind(GL_ARRAY_BUFFER);
    glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(float), vertexData.data(), GL_STATIC_DRAW);

    _indexBuffer.create();
    _indexBuffer.bind(GL_ELEMENT_ARRAY_BUFFER);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexData.size() * sizeof(uint16_t), indexData.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 9, nullptr);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 9, (GLvoid*)(sizeof(float) * 3));
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 9, (GLvoid*)(sizeof(float) * 6));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);

    _indexCount = indexData.size();

    initTerrainTexture();
    initBlendTexture();
}

LandblockRenderer::~LandblockRenderer()
{
    cleanupTerrainTexture();
    cleanupBlendTexture();

    _vertexBuffer.destroy();
    _indexBuffer.destroy();
}

void LandblockRenderer::render()
{
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D_ARRAY, _terrainTexture);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D_ARRAY, _blendTexture);

    _vertexBuffer.bind(GL_ARRAY_BUFFER);
    _indexBuffer.bind(GL_ELEMENT_ARRAY_BUFFER);
    glDrawElements(GL_TRIANGLE_STRIP, _indexCount, GL_UNSIGNED_SHORT, nullptr);
}

void LandblockRenderer::initTerrainTexture()
{
    // allocate terrain texture
    glGenTextures(1, &_terrainTexture);
    glBindTexture(GL_TEXTURE_2D_ARRAY, _terrainTexture);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGB8, TERRAIN_ARRAY_SIZE, TERRAIN_ARRAY_SIZE, TERRAIN_ARRAY_DEPTH, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);

    // populate terrain texture 
    for(int i = 0; i < TERRAIN_ARRAY_DEPTH; i++)
    {
        Image image;
        image.load(LANDSCAPE_TEXTURES[i]);

        if(image.width() != TERRAIN_ARRAY_SIZE || image.height() != TERRAIN_ARRAY_SIZE)
        {
            throw runtime_error("Bad terrain image size");
        }

        if(image.format() != Image::RGB24)
        {
            throw runtime_error("Bad terrain image format");
        }

        glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, i, TERRAIN_ARRAY_SIZE, TERRAIN_ARRAY_SIZE, 1, GL_RGB, GL_UNSIGNED_BYTE, image.data());
    }
}

void LandblockRenderer::cleanupTerrainTexture()
{
    glDeleteTextures(1, &_terrainTexture);
}

void LandblockRenderer::initBlendTexture()
{
    // allocate terrain texture
    glGenTextures(1, &_blendTexture);
    glBindTexture(GL_TEXTURE_2D_ARRAY, _blendTexture);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_R8, TERRAIN_ARRAY_SIZE, TERRAIN_ARRAY_SIZE, TERRAIN_ARRAY_DEPTH, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr);

    // populate terrain texture 
    for(int i = 0; i < BLEND_ARRAY_DEPTH; i++)
    {
        Image image;
        image.load(BLEND_TEXTURES[i]);

        if(image.width() != BLEND_ARRAY_SIZE || image.height() != BLEND_ARRAY_SIZE)
        {
            throw runtime_error("Bad terrain image size");
        }

        if(image.format() != Image::A8)
        {
            throw runtime_error("Bad terrain image format");
        }

        glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, i, BLEND_ARRAY_SIZE, BLEND_ARRAY_SIZE, 1, GL_RED, GL_UNSIGNED_BYTE, image.data());
    }
}

void LandblockRenderer::cleanupBlendTexture()
{
    glDeleteTextures(1, &_blendTexture);
}
