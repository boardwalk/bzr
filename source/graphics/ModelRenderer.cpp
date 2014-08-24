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
#include "math/Vec4.h"
#include "Camera.h"
#include "Core.h"
#include "LandblockManager.h"
#include "Model.h"
#include "ModelGroup.h"
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
        return _cameraPos.squareDist(a.worldPos) > _cameraPos.squareDist(b.worldPos);
    }

    Vec3 _cameraPos;
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

void ModelRenderer::render(const Mat4& projectionMat, const Mat4& viewMat)
{
    _program.use();

    auto& landblockManager = Core::get().landblockManager();

    auto cameraPosition = Core::get().camera().position();
    glUniform4f(_program.getUniform("cameraPosition"), GLfloat(cameraPosition.x), GLfloat(cameraPosition.y), GLfloat(cameraPosition.z), 1.0f);

    // first pass, render solid objects and collect objects that need depth sorting
    _depthSortList.clear();

    for(auto& pair : landblockManager)
    {
        auto dx = pair.first.x() - landblockManager.center().x();
        auto dy = pair.first.y() - landblockManager.center().y();

        auto landblockPosition = Vec3(dx * 192.0, dy * 192.0, 0.0);

        for(auto& object : pair.second.objects())
        {
            Mat4 worldTransMat;
            worldTransMat.makeTranslation(landblockPosition + object.position);

            Mat4 worldRotMat;
            worldRotMat.makeRotation(object.rotation);

            auto worldMat = worldTransMat * worldRotMat;

            renderOne(const_cast<ResourcePtr&>(object.resource),
                projectionMat,
                viewMat,
                worldMat);
        }

        for(auto& structure : pair.second.structures())
        {
            for(auto& object : structure.objects())
            {
                Mat4 worldTransMat;
                worldTransMat.makeTranslation(landblockPosition + object.position);

                Mat4 worldRotMat;
                worldRotMat.makeRotation(object.rotation);

                auto worldMat = worldTransMat * worldRotMat;

                renderOne(const_cast<ResourcePtr&>(object.resource),
                    projectionMat,
                    viewMat,
                    worldMat);
            }
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

void ModelRenderer::renderOne(ResourcePtr& resource,
    const Mat4& projectionMat,
    const Mat4& viewMat, 
    const Mat4& worldMat)
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

void ModelRenderer::renderModelGroup(ModelGroup& modelGroup,
    const Mat4& projectionMat,
    const Mat4& viewMat,
    const Mat4& worldMat)
{
    for(auto& modelInfo : modelGroup.modelInfos())
    {
        Mat4 subWorldTransMat;
        subWorldTransMat.makeTranslation(modelInfo.position);

        Mat4 subWorldRotMat;
        subWorldRotMat.makeRotation(modelInfo.rotation);

        Mat4 subWorldScaleMat;
        subWorldScaleMat.makeScale(modelInfo.scale);

        auto subWorldMat = subWorldTransMat * subWorldRotMat * subWorldScaleMat;

        // fix position, rotation!
        renderOne(const_cast<ResourcePtr&>(modelInfo.resource),
            projectionMat,
            viewMat,
            worldMat * subWorldMat);
    }
}

void ModelRenderer::renderModel(Model& model,
    const Mat4& projectionMat,
    const Mat4& viewMat,
    const Mat4& worldMat,
    bool firstPass)
{
    if(firstPass && model.needsDepthSort())
    {
        auto worldPos = worldMat * Vec4(0.0, 0.0, 0.0, 1.0);
        DepthSortedModel depthSortedModel = { &model, worldMat, Vec3(worldPos.x, worldPos.y, worldPos.z) };
        _depthSortList.push_back(depthSortedModel);
        return;
    }

    auto worldViewProjectionMat = projectionMat * viewMat * worldMat;

    loadMat4ToUniform(worldMat, _program.getUniform("worldMatrix"));
    loadMat4ToUniform(worldViewProjectionMat, _program.getUniform("worldViewProjectionMatrix"));

    if(!model.renderData())
    {
        model.renderData().reset(new MeshRenderData(model));
    }

    auto& renderData = (MeshRenderData&)*model.renderData();

    renderData.render();
}
