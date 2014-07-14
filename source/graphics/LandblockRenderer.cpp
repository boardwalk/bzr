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
    0x0600383f,
    0x0600383f,
    0x0600383f,
    0x0600383f,
    0x0600383f,
    0x0600383f,
    0x0600383f,
    0x0600383f,
    0x0600383f,
    0x0600383f,
    0x0600383f,
    0x0600383f,
    0x0600383f,
    0x0600383f,
    0x0600383f,
    0x0600383f,
    0x0600383f,
    0x0600383f,
    0x0600383f,
    0x0600383f,
    0x0600383f,
    0x0600383f,
    0x0600383f,
    0x0600383f,
    0x0600383f,
    0x0600383f,
    0x0600383f,
    0x0600383f,
    0x0600383f,
    0x0600383f,
    0x0600383f,
    0x0600383f
};

static const int TERRAIN_ARRAY_SIZE = 256;
static const int TERRAIN_ARRAY_DEPTH = sizeof(LANDSCAPE_TEXTURES) / sizeof(LANDSCAPE_TEXTURES[0]);

// blend textures
// 6d61 -- vertical, black to white, left of center
// 6d6c -- top left corner, black, semi ragged
// 6d6d -- top left corner, black, ragged
// 6d60 -- top left corner, black, rounded
// 37fd -- top left corner, black, puzzle piece
// 6d30 -- vertical, black to white, very left of center, wavy

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

            //Vec3 we(Landblock::LANDBLOCK_SIZE / double(size - 1) * 2.0, 0.0, he - hw);
            //Vec3 sn(0.0, Landblock::LANDBLOCK_SIZE / double(size - 1) * 2.0, hn - hs);
            //auto n = we.cross(sn).normalize();

            //printf("we:(%f, %f, %f) sn:(%f, %f, %f), n:(%f, %f, %f)\n",
            //    we.x, we.y, we.z,
            //    sn.x, sn.y, sn.z,
            //    n.x, n.y, n.z);

            // x, y, z
            vertexData.push_back(double(x) / double(size - 1) * Landblock::LANDBLOCK_SIZE);
            vertexData.push_back(double(y) / double(size - 1) * Landblock::LANDBLOCK_SIZE);
            vertexData.push_back(data[x + y * size]);
            // nx, ny, nz
            vertexData.push_back(n.x);
            vertexData.push_back(n.y);
            vertexData.push_back(n.z);
            // tx, ty, tz
            vertexData.push_back(double(x) / double(size - 1) * (Landblock::GRID_SIZE - 1));
            vertexData.push_back(double(y) / double(size - 1) * (Landblock::GRID_SIZE - 1));
            vertexData.push_back(0.0);
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
}

LandblockRenderer::~LandblockRenderer()
{
    cleanupTerrainTexture();

    _vertexBuffer.destroy();
    _indexBuffer.destroy();
}

void LandblockRenderer::render()
{
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D_ARRAY, _terrainTexture);

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
        printf("loading %d\n", LANDSCAPE_TEXTURES[i]);

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
