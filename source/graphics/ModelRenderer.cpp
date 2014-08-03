#include "graphics/ModelRenderer.h"
#include "graphics/ModelRenderData.h"
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

void ModelRenderer::render()
{
    _program.use();

    auto& model = (SimpleModel&)*_model;

    auto& renderData = model.renderData();

    if(!renderData)
    {
        renderData.reset(new ModelRenderData(model));
    }

    auto& modelRenderData = (ModelRenderData&)*renderData;

    modelRenderData.render();
}
