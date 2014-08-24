#include "graphics/ModelRenderer.h"
#include "graphics/MeshRenderData.h"
#include "graphics/Renderer.h"
#include "graphics/TextureAtlas.h"
#include "graphics/util.h"
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
        // FIXME This is total bullshit right now
        // descending order
        //return _cameraPos.squareDist(a.worldPos) > _cameraPos.squareDist(b.worldPos);
        return a.worldMat.m[0] < b.worldMat.m[0];
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

    auto atlasLocation = _program.getUniform("atlas");
    glUniform1i(atlasLocation, 0);

    auto atlasTocLocation = _program.getUniform("atlasToc");
    glUniform1i(atlasTocLocation, 1);
}

ModelRenderer::~ModelRenderer()
{
    _program.destroy();
}

void ModelRenderer::render(const Mat4& projectionMat, const Mat4& viewMat)
{
    _program.use();

    Core::get().renderer().textureAtlas().bind();

    auto& landblockManager = Core::get().landblockManager();

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
        DepthSortedModel depthSortedModel = { &model, worldMat };
        _depthSortList.push_back(depthSortedModel);
        return;
    }

    auto modelViewProjectionMat = projectionMat * viewMat * worldMat;

    loadMat4ToUniform(modelViewProjectionMat, _program.getUniform("modelViewProjectionMatrix"));

    if(!model.renderData())
    {
        model.renderData().reset(new MeshRenderData(model));
    }

    auto& renderData = (MeshRenderData&)*model.renderData();

    renderData.bind();

    glDrawElements(GL_TRIANGLE_FAN, renderData.indexCount(), GL_UNSIGNED_SHORT, nullptr);
}
