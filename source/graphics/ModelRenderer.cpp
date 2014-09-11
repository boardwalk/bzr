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
#include "graphics/ModelRenderer.h"
#include "graphics/MeshRenderData.h"
#include "graphics/Renderer.h"
#include "graphics/util.h"
#include "Camera.h"
#include "Core.h"
#include "Land.h"
#include "LandcellManager.h"
#include "Model.h"
#include "ModelGroup.h"
#include "ObjectManager.h"
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>

#include "graphics/shaders/ModelVertexShader.h"
#include "graphics/shaders/ModelFragmentShader.h"

struct CompareByDepth
{
    CompareByDepth()
    {
        _cameraPos = Core::get().camera().position();
    }

    bool operator()(const ModelRenderer::DepthSortedModel& a, const ModelRenderer::DepthSortedModel& b) const
    {
        // descending order
        return (_cameraPos - a.worldPos).length() > (_cameraPos - b.worldPos).length();
    }

    glm::vec3 _cameraPos;
};

ModelRenderer::ModelRenderer()
{
    _program.create();
    _program.attach(GL_VERTEX_SHADER, ModelVertexShader);
    _program.attach(GL_FRAGMENT_SHADER, ModelFragmentShader);
    _program.link();

    _program.use();

    auto texLocation = _program.getUniform("tex");
    glUniform1i(texLocation, 0);
}

ModelRenderer::~ModelRenderer()
{
    _program.destroy();
}

void ModelRenderer::render(const glm::mat4& projectionMat, const glm::mat4& viewMat)
{
    _program.use();

    auto& landcellManager = Core::get().landcellManager();
    auto& objectManager = Core::get().objectManager();

    auto cameraPosition = Core::get().camera().position();
    glUniform4f(_program.getUniform("cameraPosition"), GLfloat(cameraPosition.x), GLfloat(cameraPosition.y), GLfloat(cameraPosition.z), 1.0f);

    // first pass, render solid objects and collect objects that need depth sorting
    _depthSortList.clear();

    for(auto& pair : objectManager)
    {
        auto& object = *pair.second;

        if(!object.model())
        {
            continue;
        }

        auto dx = object.location().landcell.x() - landcellManager.center().x();
        auto dy = object.location().landcell.y() - landcellManager.center().y();

        auto blockPosition = glm::vec3(dx * Land::BLOCK_SIZE, dy * Land::BLOCK_SIZE, 0.0);

        auto worldMat = glm::translate(glm::mat4(), blockPosition + object.location().offset) * glm::mat4_cast(object.location().rotation);

        renderOne(object.model(), projectionMat, viewMat, worldMat);
    }

    for(auto& pair : landcellManager)
    {
        auto dx = pair.first.x() - landcellManager.center().x();
        auto dy = pair.first.y() - landcellManager.center().y();

        auto blockPosition = glm::vec3(dx * Land::BLOCK_SIZE, dy * Land::BLOCK_SIZE, 0.0);

        for(auto& doodad : pair.second->doodads())
        {
            auto worldMat = glm::translate(glm::mat4(), blockPosition + doodad.position) * glm::mat4_cast(doodad.rotation);

            renderOne(doodad.resource, projectionMat, viewMat, worldMat);
        }
    }

    // second pass, sort and render objects that need depth sorting
    sort(_depthSortList.begin(), _depthSortList.end(), CompareByDepth());

    for(auto& depthSortedModel : _depthSortList)
    {
        renderModel(*depthSortedModel.model,
            projectionMat,
            viewMat,
            depthSortedModel.worldMat,
            /*firstPass*/ false);
    }
}

void ModelRenderer::renderOne(const ResourcePtr& resource,
    const glm::mat4& projectionMat,
    const glm::mat4& viewMat, 
    const glm::mat4& worldMat)
{
    if(resource->resourceType() == ResourceType::ModelGroup)
    {
        renderModelGroup(resource->cast<ModelGroup>(),
            projectionMat,
            viewMat,
            worldMat);
    }
    else if(resource->resourceType() == ResourceType::Model)
    {
        renderModel(resource->cast<Model>(),
            projectionMat,
            viewMat,
            worldMat,
            /*firstPass*/ true);
    }
}

void ModelRenderer::renderModelGroup(const ModelGroup& modelGroup,
    const glm::mat4& projectionMat,
    const glm::mat4& viewMat,
    const glm::mat4& worldMat)
{
    for(auto child = 0u; child < modelGroup.modelInfos().size(); child++)
    {
        auto& modelInfo = modelGroup.modelInfos()[child];

        auto subWorldMat = glm::translate(glm::mat4(), modelInfo.position) * glm::mat4_cast(modelInfo.rotation) * glm::scale(glm::mat4(), modelInfo.scale);

        renderOne(modelInfo.resource,
            projectionMat,
            viewMat,
            worldMat * subWorldMat);
    }
}

void ModelRenderer::renderModel(const Model& model,
    const glm::mat4& projectionMat,
    const glm::mat4& viewMat,
    const glm::mat4& worldMat,
    bool firstPass)
{
    if(firstPass && model.needsDepthSort())
    {
        auto worldPos = worldMat * glm::vec4(0.0, 0.0, 0.0, 1.0);
        DepthSortedModel depthSortedModel = { &model, worldMat, glm::vec3(worldPos.x, worldPos.y, worldPos.z) };
        _depthSortList.push_back(depthSortedModel);
        return;
    }

    loadMat4ToUniform(worldMat, _program.getUniform("worldMatrix"));
    loadMat4ToUniform(viewMat, _program.getUniform("viewMatrix"));
    loadMat4ToUniform(projectionMat, _program.getUniform("projectionMatrix"));

    if(!model.renderData())
    {
        model.renderData().reset(new MeshRenderData(model));
    }

    auto& renderData = (MeshRenderData&)*model.renderData();

    renderData.render();
}
