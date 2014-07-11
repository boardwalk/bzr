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
    /*
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
    */

    for(auto y = 0u; y < size - 1; y++)
    {
        for(auto x = 0u; x < size - 1; x++)
        {
            // 4--3
            // |  |
            // 1--2
            auto h1 = vertexData[x + y * size];
            auto h2 = vertexData[(x + 1) + y * size];
            auto h3 = vertexData[(x + 1) + y * (size + 1)];
            auto h4 = vertexData[x + y * (size + 1)];

            auto d13 = fabs(h3 - h1);
            auto d24 = fabs(h4 - h2);

            if(d13 > d24)
            {
                indexData.push_back(x + y * size); // 1
                indexData.push_back((x + 1) + y * size); // 2
                indexData.push_back((x + 1) + (y + 1) * size); // 3

                indexData.push_back(x + y * size); // 1
                indexData.push_back((x + 1) + (y + 1) * size); // 3
                indexData.push_back(x + (y + 1) * size); // 4
            }
            else
            {
                indexData.push_back(x + y * size); // 1
                indexData.push_back((x + 1) + y * size); // 2
                indexData.push_back(x + (y + 1) * size); // 4

                indexData.push_back((x + 1) + y * size); // 2
                indexData.push_back((x + 1) + (y + 1) * size); // 3
                indexData.push_back(x + (y + 1) * size); // 4
            }
        }
    }

    assert(indexData.size() < 0xffff);

    glGenBuffers(1, &_vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, _vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(float), vertexData.data(), GL_STATIC_DRAW);

    glGenBuffers(1, &_indexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _indexBuffer);
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
    glDeleteBuffers(1, &_vertexBuffer);
    glDeleteBuffers(1, &_indexBuffer);
}

void LandblockRenderer::render()
{
    _texture.bind(0);

    glBindBuffer(GL_ARRAY_BUFFER, _vertexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _indexBuffer);
    //glDrawElements(GL_TRIANGLE_STRIP, _indexCount, GL_UNSIGNED_SHORT, nullptr);
    glDrawElements(GL_TRIANGLES, _indexCount, GL_UNSIGNED_SHORT, nullptr);
}

