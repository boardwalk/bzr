/*
 * Bael'Zharon's Respite
 * Copyright (C) 2014 Daniel Skorupski
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */
#include "graphics/MeshRenderData.h"
#include "graphics/Renderer.h"
#include "graphics/TextureAtlas.h"
#include "Core.h"
#include "Model.h"
#include "Structure.h"
#include "StructureGeom.h"

MeshRenderData::MeshRenderData(const Model& model)
{
    initGeometry(model.textures(), model.vertices(), model.triangleFans());
}

MeshRenderData::MeshRenderData(const Structure& structure)
{
    assert(structure.pieceNum() < structure.geometry().pieces().size());
    auto& piece = structure.geometry().pieces()[structure.pieceNum()];
    initGeometry(structure.textures(), piece.vertices, piece.triangleFans);
}

MeshRenderData::~MeshRenderData()
{
    glDeleteVertexArrays(1, &_vertexArray);
    glDeleteBuffers(1, &_vertexBuffer);
    glDeleteBuffers(1, &_indexBuffer);
}

void MeshRenderData::bind()
{
    glBindVertexArray(_vertexArray);
}

GLsizei MeshRenderData::indexCount() const
{
    return _indexCount;
}

void MeshRenderData::initGeometry(const vector<ResourcePtr>& textures, const vector<Vertex>& vertices, const vector<TriangleFan>& triangleFans)
{
    // vx, vy, vz, nx, ny, nz, s, t, p
    static const int COMPONENTS_PER_VERTEX = 9;

    vector<int> textureIndices;

    for(auto& texture : textures)
    {
        textureIndices.push_back(Core::get().renderer().textureAtlas().get(texture->resourceId()));
    }

    // we're duplicating a bit to make all this fit the structure of modern graphics APIs
    // the models are so low resolution it should not matter

    vector<float> vertexData;
    vector<uint16_t> indexData;

    for(auto& triangleFan : triangleFans)
    {
        if(!indexData.empty())
        {
            indexData.push_back(0xFFFF);
        }
        
        for(auto& index : triangleFan.indices)
        {
            indexData.push_back(uint16_t(vertexData.size() / COMPONENTS_PER_VERTEX));

            auto& vertex = vertices[index.vertexIndex];

            vertexData.push_back(float(vertex.position.x));
            vertexData.push_back(float(vertex.position.y));
            vertexData.push_back(float(vertex.position.z));

            vertexData.push_back(float(vertex.normal.x));
            vertexData.push_back(float(vertex.normal.y));
            vertexData.push_back(float(vertex.normal.z));

            if(vertex.texCoords.empty())
            {
                vertexData.push_back(1.0f);
                vertexData.push_back(1.0f);
            }
            else
            {
                vertexData.push_back(1.0f - float(vertex.texCoords[index.texCoordIndex].x));
                vertexData.push_back(1.0f - float(vertex.texCoords[index.texCoordIndex].y));
            }

            assert((size_t)triangleFan.texIndex < textureIndices.size());
            vertexData.push_back((float)textureIndices[triangleFan.texIndex]);
        }
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
