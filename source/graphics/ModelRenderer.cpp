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
            renderOne(const_cast<ResourcePtr&>(object.resource), projectionMat, viewMat, landblockPosition + object.position, object.rotation);
        }

        for(auto& structure : pair.second.structures())
        {
            // TODO apply structure rotation
            auto structurePosition = landblockPosition + structure.position();

            for(auto& object : structure.objects())
            {
                renderOne(const_cast<ResourcePtr&>(object.resource), projectionMat, viewMat, structurePosition + object.position, object.rotation);
            }
        }
    }

    // second pass, sort and render objects that need depth sorting
    sort(_depthSortList.begin(), _depthSortList.end(), CompareByDepth());

    for(auto& depthSortedModel : _depthSortList)
    {
        renderModel(*depthSortedModel.model, projectionMat, viewMat, depthSortedModel.worldPos, depthSortedModel.worldRot, /*firstPass*/ false);
    }
}

void ModelRenderer::renderOne(ResourcePtr& resource, const Mat4& projectionMat, const Mat4& viewMat, const Vec3& position, const Quat& rotation)
{
    if(resource->resourceType() == ResourceType::ModelGroup)
    {
        renderModelGroup(resource->cast<ModelGroup>(), 0xFFFFFFFF, projectionMat, viewMat, position, rotation);
    }
    else if(resource->resourceType() == ResourceType::Model)
    {
        renderModel(resource->cast<Model>(), projectionMat, viewMat, position, rotation, /*firstPass*/ true);
    }
}

void ModelRenderer::renderModelGroup(ModelGroup& modelGroup, uint32_t parent, const Mat4& projectionMat, const Mat4& viewMat, const Vec3& position, const Quat& rotation)
{
    for(auto i = 0u; i < modelGroup.modelInfos().size(); i++)
    {
        auto& modelInfo = modelGroup.modelInfos()[i];

        if(modelInfo.parent == parent)
        {
            // fix position, rotation!
            renderOne(const_cast<ResourcePtr&>(modelInfo.resource), projectionMat, viewMat, position + modelInfo.position, rotation);
            renderModelGroup(modelGroup, i, projectionMat, viewMat, position + modelInfo.position, rotation);
        }
    }
}

void ModelRenderer::renderModel(Model& model, const Mat4& projectionMat, const Mat4& viewMat, const Vec3& position, const Quat& rotation, bool firstPass)
{
    if(firstPass && model.needsDepthSort())
    {
        DepthSortedModel depthSortedModel = { &model, position, rotation };
        _depthSortList.push_back(depthSortedModel);
        return;
    }

    Mat4 modelRotationMat;
    modelRotationMat.makeRotation(rotation);

    Mat4 modelTranslationMat;
    modelTranslationMat.makeTranslation(position);

    auto modelMat = modelTranslationMat * modelRotationMat;
    auto modelViewProjectionMat = projectionMat * viewMat * modelMat;

    loadMat4ToUniform(modelViewProjectionMat, _program.getUniform("modelViewProjectionMatrix"));

    if(!model.renderData())
    {
        model.renderData().reset(new MeshRenderData(model));
    }

    auto& renderData = (MeshRenderData&)*model.renderData();

    renderData.bind();

    glDrawElements(GL_TRIANGLE_FAN, renderData.indexCount(), GL_UNSIGNED_SHORT, nullptr);
}
