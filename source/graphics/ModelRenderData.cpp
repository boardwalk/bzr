#include "graphics/ModelRenderData.h"
#include "SimpleModel.h"
#include <vector>

ModelRenderData::ModelRenderData(const SimpleModel& model)
{
    vector<float> vertexData;

    for(auto& vert : model.vertices())
    {
        vertexData.push_back(float(vert.position.x));
        vertexData.push_back(float(vert.position.y));
        vertexData.push_back(float(vert.position.z));

        vertexData.push_back(float(vert.normal.x));
        vertexData.push_back(float(vert.normal.y));
        vertexData.push_back(float(vert.normal.z));
    }

    vector<uint16_t> indexData;

    for(auto& prim : model.primitives())
    {
        for(auto index : prim.vertexIndices)
        {
            assert(size_t(index) < model.vertices().size());
            indexData.push_back(uint16_t(index));
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

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 6, nullptr);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 6, (GLvoid*)(sizeof(float) * 3));

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
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
