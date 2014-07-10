#include "gfx/LandblockRenderer.h"
#include "Landblock.h"
#include <vector>

// TODO We could only create one indexBuffer per subdivision level
// TODO We could generate the x and y of the vertex data in a shader

LandblockRenderer::LandblockRenderer(const Landblock& landblock)
{
    auto data = landblock.getSubdividedData();
    auto size = landblock.getSubdividedSize();

    vector<double> vertexData;

    for(auto y = 0u; y < size; y++)
    {
        for(auto x = 0u; x < size; x++)
        {
            vertexData.push_back(double(x) / double(size) * Landblock::LANDBLOCK_SIZE);
            vertexData.push_back(double(y) / double(size) * Landblock::LANDBLOCK_SIZE);
            vertexData.push_back(data[x + y * size]);
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

    glGenBuffers(1, &_vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, _vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(double), vertexData.data(), GL_STATIC_DRAW);

    glGenBuffers(1, &_indexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _indexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexData.size() * sizeof(uint16_t), indexData.data(), GL_STATIC_DRAW);

    _indexCount = indexData.size();
}

LandblockRenderer::~LandblockRenderer()
{
    glDeleteBuffers(1, &_vertexBuffer);
    glDeleteBuffers(1, &_indexBuffer);
}

void LandblockRenderer::render()
{
    glBindBuffer(GL_ARRAY_BUFFER, _vertexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _indexBuffer);
    glDrawElements(GL_TRIANGLE_STRIP, _indexCount, GL_UNSIGNED_SHORT, nullptr);
}

