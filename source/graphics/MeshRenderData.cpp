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
#include "graphics/TextureRenderData.h"
#include "Core.h"
#include "Config.h"
#include "Model.h"
#include "ResourceCache.h"
#include "Structure.h"
#include "StructureGeom.h"
#include "Texture.h"
#include "TextureLookup5.h"
#include "TextureLookup8.h"
#include <algorithm>

// FIXME Not the neatest thing in the world
// Ought to find an existing 0x08 file with a nice transparent texture
static weak_ptr<Resource> hitTexture;

struct SortByTexIndex
{
    bool operator()(const TriangleFan* a, const TriangleFan* b)
    {
        return a->texIndex < b->texIndex;
    }
};

MeshRenderData::MeshRenderData(const Model& model)
{
    init(model.textures(),
        model.vertices(),
        model.triangleFans(),
        model.hitTriangleFans());
}

MeshRenderData::MeshRenderData(const Structure& structure)
{
    assert(structure.partNum() < structure.geometry().size());
    auto& part = structure.geometry()[structure.partNum()];

    init(structure.textures(),
        part.vertices(),
        part.triangleFans(),
        part.hitTriangleFans());
}

MeshRenderData::~MeshRenderData()
{
    glDeleteVertexArrays(1, &_vertexArray);
    glDeleteBuffers(1, &_vertexBuffer);
    glDeleteBuffers(1, &_indexBuffer);
}

void MeshRenderData::render()
{
    glBindVertexArray(_vertexArray);

    auto indexBase = 0;

    for(auto& batch : _batches)
    {
        auto& texture = const_cast<Texture&>(batch.texture->cast<TextureLookup8>().textureLookup5().texture());

        if(!texture.renderData())
        {
            texture.renderData().reset(new TextureRenderData(texture));
        }

        auto& renderData = (TextureRenderData&)*texture.renderData();

        glActiveTexture(GL_TEXTURE0);
        renderData.bind();

        glDrawElements(GL_TRIANGLE_FAN, batch.indexCount, GL_UNSIGNED_SHORT, (void*)(indexBase * sizeof(uint16_t)));

        indexBase += batch.indexCount;
    }
}

void MeshRenderData::init(
    const vector<ResourcePtr>& textures,
    const vector<Vertex>& vertices,
    const vector<TriangleFan>& triangleFans,
    const vector<TriangleFan>& hitTriangleFans)
{
    // vx, vy, vz, nx, ny, nz, s, t
    static const int COMPONENTS_PER_VERTEX = 8;

    // Sort triangle fans by texture
    vector<const TriangleFan*> sortedTriangleFans;

    for(auto& triangleFan : triangleFans)
    {
        sortedTriangleFans.push_back(&triangleFan);
    }

    sort(sortedTriangleFans.begin(), sortedTriangleFans.end(), SortByTexIndex());

    // Build batches
    vector<float> vertexData;
    vector<uint16_t> indexData;

    for(auto triangleFan : sortedTriangleFans)
    {
        // Skip portal/lighting polygons
        if(triangleFan->flags == 0x04)
        {
            continue;
        }

        if(_batches.empty() || textures[triangleFan->texIndex].get() != _batches.back().texture.get())
        {
            // Start a new batch
            Batch batch = { textures[triangleFan->texIndex], 0 };
            _batches.push_back(batch);
        }
        else if(_batches.back().indexCount != 0)
        {
            // Starting a new triangle fan in existing batch
            indexData.push_back(0xFFFF);
            _batches.back().indexCount++;
        }

        for(auto& index : triangleFan->indices)
        {
            indexData.push_back(uint16_t(vertexData.size() / COMPONENTS_PER_VERTEX));
            _batches.back().indexCount++;

            auto& vertex = vertices[index.vertexIndex];

            vertexData.push_back(float(vertex.position.x));
            vertexData.push_back(float(vertex.position.y));
            vertexData.push_back(float(vertex.position.z));

            vertexData.push_back(float(vertex.normal.x));
            vertexData.push_back(float(vertex.normal.y));
            vertexData.push_back(float(vertex.normal.z));

            if(vertex.texCoords.empty())
            {
                vertexData.push_back(0.0f);
                vertexData.push_back(0.0f);
            }
            else
            {
                vertexData.push_back(float(vertex.texCoords[index.texCoordIndex].x));
                vertexData.push_back(float(vertex.texCoords[index.texCoordIndex].y));
            }
        }
    }

    if(Core::get().config().getBool("MeshRenderData.renderHitGeometry", false))
    {
        auto textureLookup8 = hitTexture.lock();

        if(!textureLookup8)
        {
            ResourcePtr texture(new Texture(0x800000FF));
            ResourcePtr textureLookup5(new TextureLookup5(texture));
            textureLookup8.reset(new TextureLookup8(textureLookup5));
            hitTexture = textureLookup8;
        }

        Batch batch = { textureLookup8, 0 };
        _batches.push_back(batch);

        for(auto& triangleFan : hitTriangleFans)
        {
            if(_batches.back().indexCount != 0)
            {
                indexData.push_back(0xFFFF);
                _batches.back().indexCount++;
            }

            for(auto& index : triangleFan.indices)
            {
                indexData.push_back(uint16_t(vertexData.size() / COMPONENTS_PER_VERTEX));
                _batches.back().indexCount++;

                auto& vertex = vertices[index.vertexIndex];

                vertexData.push_back(float(vertex.position.x));
                vertexData.push_back(float(vertex.position.y));
                vertexData.push_back(float(vertex.position.z));

                vertexData.push_back(float(vertex.normal.x));
                vertexData.push_back(float(vertex.normal.y));
                vertexData.push_back(float(vertex.normal.z));

                vertexData.push_back(0.0f);
                vertexData.push_back(0.0f);
            }
        }
    }

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
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(float) * COMPONENTS_PER_VERTEX, (GLvoid*)(sizeof(float) * 6));

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
}
