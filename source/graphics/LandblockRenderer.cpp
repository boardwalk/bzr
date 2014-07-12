#include "graphics/LandblockRenderer.h"
#include "graphics/loadTexture.h"
#include "Landblock.h"
#include <vector>

// TODO We could only create one indexBuffer per subdivision level
// TODO We could generate the x and y of the vertex data in a shader

LandblockRenderer::LandblockRenderer(const Landblock& landblock)
{
    auto data = landblock.getSubdividedData();
    auto size = landblock.getSubdividedSize();

    vector<float> vertexData;

    for(auto y = 0u; y < size; y++)
    {
        for(auto x = 0u; x < size; x++)
        {
            // x, y, z
            vertexData.push_back(double(x) / double(size) * Landblock::LANDBLOCK_SIZE);
            vertexData.push_back(double(y) / double(size) * Landblock::LANDBLOCK_SIZE);
            vertexData.push_back(data[x + y * size]);
            // s, t
            vertexData.push_back(double(x) / double(size));
            vertexData.push_back(double(y) / double(size));
        }
    }

    vector<uint16_t> indexData;

    // B-D-F
    // |\|\| ...
    // A-C-E
    // http://en.wikipedia.org/wiki/Triangle_strip
    for(auto y = 0u; y < size - 1; y++)
    {
        if(y != 0)
        {
            // end old triangle strip
            indexData.push_back(0xffff);
        }

        auto x = 0u;
        
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

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 5, nullptr);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 5, (GLvoid*)(sizeof(float) * 3));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    _indexCount = indexData.size();

    _texture = loadTexture(0x0600383f);
}

LandblockRenderer::~LandblockRenderer()
{
    _vertexBuffer.destroy();
    _indexBuffer.destroy();
}

void LandblockRenderer::render()
{
    _texture.bind(0);

    _vertexBuffer.bind(GL_ARRAY_BUFFER);
    _indexBuffer.bind(GL_ELEMENT_ARRAY_BUFFER);
    glDrawElements(GL_TRIANGLE_STRIP, _indexCount, GL_UNSIGNED_SHORT, nullptr);
}
