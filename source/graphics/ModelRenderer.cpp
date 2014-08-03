#include "graphics/ModelRenderer.h"
#include "graphics/ModelRenderData.h"
#include "graphics/util.h"
#include "math/Mat4.h"
#include "Core.h"
#include "ResourceCache.h"
#include "SimpleModel.h"

#include "graphics/shaders/ModelVertexShader.h"
#include "graphics/shaders/ModelFragmentShader.h"

ModelRenderer::ModelRenderer()
{
    _program.create();
    _program.attach(GL_VERTEX_SHADER, ModelVertexShader);
    _program.attach(GL_FRAGMENT_SHADER, ModelFragmentShader);
    _program.link();

    _model = Core::get().resourceCache().get(0x01000007);
}

ModelRenderer::~ModelRenderer()
{
    _model.reset();

    _program.destroy();
}

void ModelRenderer::render(const Mat4& projectionMat, const Mat4& viewMat)
{
    _program.use();

    Mat4 modelMat;
    auto modelViewMat = viewMat * modelMat;
    auto modelViewProjectionMat = projectionMat * modelViewMat;

    loadMat4ToUniform(modelViewProjectionMat, _program.getUniform("modelViewProjectionMatrix"));

    auto& model = (SimpleModel&)*_model;

    auto& renderData = model.renderData();

    if(!renderData)
    {
        renderData.reset(new ModelRenderData(model));
    }

    auto& modelRenderData = (ModelRenderData&)*renderData;

    modelRenderData.bind();

    glDrawElements(GL_TRIANGLE_STRIP, modelRenderData.indexCount(), GL_UNSIGNED_SHORT, nullptr);
}
