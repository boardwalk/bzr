#include "graphics/ModelRenderData.h"
#include "Model.h"
#include <vector>

ModelRenderData::ModelRenderData(const Model& model)
{
    // vx, vy, vz, nx, ny, nz, tx, ty
    static const int COMPONENTS_PER_VERTEX = 8;

    uint16_t newIndex = 0;

    vector<float> vertexData;

    vector<vector<uint16_t>> newIndices;
    newIndices.resize(model.vertices().size());

    for(auto vi = 0u; vi < model.vertices().size(); vi++)
    {
        auto& vert = model.vertices()[vi];

        newIndices[vi].resize(vert.texCoords.size());

        for(auto vti = 0u; vti < vert.texCoords.size(); vti++)
        {
            newIndices[vi][vti] = newIndex++;

            vertexData.push_back(float(vert.position.x));
            vertexData.push_back(float(vert.position.y));
            vertexData.push_back(float(vert.position.z));

            vertexData.push_back(float(vert.normal.x));
            vertexData.push_back(float(vert.normal.y));
            vertexData.push_back(float(vert.normal.z));

            vertexData.push_back(float(vert.texCoords[vti].x));
            vertexData.push_back(float(vert.texCoords[vti].y));
        }

        if(vert.texCoords.empty())
        {
            newIndices[vi].push_back(newIndex++);

            vertexData.push_back(float(vert.position.x));
            vertexData.push_back(float(vert.position.y));
            vertexData.push_back(float(vert.position.z));

            vertexData.push_back(float(vert.normal.x));
            vertexData.push_back(float(vert.normal.y));
            vertexData.push_back(float(vert.normal.z));

            vertexData.push_back(0.0f);
            vertexData.push_back(0.0f);
        }
    }

    vector<uint16_t> indexData;

    for(auto& prim : model.primitives())
    {
        for(auto& index : prim.indices)
        {
            indexData.push_back(newIndices[index.vertexIndex][index.texCoordIndex]);
        }

        indexData.push_back(0xFFFF);
    }

    _indexCount = GLsizei(indexData.size());

    glGenVertexArrays(1, &_vertexArray);
    glBindVertexArray(_vertexArray);

    glGenBuffers(1, &_vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, _vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(float), vertexData.data(), GL_STATIC_DRAW);

    glGenBuffers(1, &_indexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _indexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexData.size() * sizeof(uint16_t), indexData.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * COMPONENTS_PER_VERTEX, nullptr);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float) * COMPONENTS_PER_VERTEX, (GLvoid*)(sizeof(float) * 3));
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(float) * COMPONENTS_PER_VERTEX, (GLvoid*)(sizeof(float) * 6));

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
}

ModelRenderData::~ModelRenderData()
{
    glDeleteVertexArrays(1, &_vertexArray);
    glDeleteBuffers(1, &_vertexBuffer);
    glDeleteBuffers(1, &_indexBuffer);
}

void ModelRenderData::bind()
{
    glBindVertexArray(_vertexArray);
}

GLsizei ModelRenderData::indexCount() const
{
    return _indexCount;
}
