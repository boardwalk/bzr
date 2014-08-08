#include "graphics/ModelRenderer.h"
#include "graphics/ModelRenderData.h"
#include "graphics/util.h"
#include "math/Mat4.h"
#include "Core.h"
#include "LandblockManager.h"
#include "Model.h"
#include "ModelGroup.h"
#include "ResourceCache.h"

#include "graphics/shaders/ModelVertexShader.h"
#include "graphics/shaders/ModelFragmentShader.h"

ModelRenderer::ModelRenderer()
{
    _program.create();
    _program.attach(GL_VERTEX_SHADER, ModelVertexShader);
    _program.attach(GL_FRAGMENT_SHADER, ModelFragmentShader);
    _program.link();

    _program.use();

    auto modelTexLocation = _program.getUniform("modelTex");
    glUniform1i(modelTexLocation, 0);
}

ModelRenderer::~ModelRenderer()
{
    _program.destroy();
}

void ModelRenderer::render(const Mat4& projectionMat, const Mat4& viewMat)
{
    _program.use();

    auto& landblockManager = Core::get().landblockManager();

    for(auto it = landblockManager.begin(); it != landblockManager.end(); ++it)
    {
        auto dx = it->first.x() - landblockManager.center().x();
        auto dy = it->first.y() - landblockManager.center().y();

        Vec3 landblockPosition(dx * 192.0, dy * 192.0, 0.0);

        for(auto& object : it->second.objects())
        {
            renderOne(const_cast<ResourcePtr&>(object.model), projectionMat, viewMat, landblockPosition + object.position, object.rotation);
        }
    }
}

void ModelRenderer::renderOne(ResourcePtr& resource, const Mat4& projectionMat, const Mat4& viewMat, const Vec3& position, const Quat& rotation)
{
    if(resource->resourceType() == Resource::ModelGroup)
    {
        renderModelGroup(resource->cast<ModelGroup>(), 0xFFFFFFFF, projectionMat, viewMat, position, rotation);
    }
    else if(resource->resourceType() == Resource::Model)
    {
        renderModel(resource->cast<Model>(), projectionMat, viewMat, position, rotation);
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

void ModelRenderer::renderModel(Model& model, const Mat4& projectionMat, const Mat4& viewMat, const Vec3& position, const Quat& rotation)
{
    Mat4 modelRotationMat;
    modelRotationMat.makeRotation(rotation);

    Mat4 modelTranslationMat;
    modelTranslationMat.makeTranslation(position);

    auto modelMat = modelTranslationMat * modelRotationMat;
    auto modelViewProjectionMat = projectionMat * viewMat * modelMat;

    loadMat4ToUniform(modelViewProjectionMat, _program.getUniform("modelViewProjectionMatrix"));

    if(!model.renderData())
    {
        model.renderData().reset(new ModelRenderData(model));
    }

    auto& renderData = (ModelRenderData&)*model.renderData();

    renderData.bind();

    glDrawElements(GL_TRIANGLE_FAN, renderData.indexCount(), GL_UNSIGNED_SHORT, nullptr);
}
