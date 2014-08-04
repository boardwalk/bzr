#include "graphics/ModelRenderer.h"
#include "graphics/ModelRenderData.h"
#include "graphics/util.h"
#include "math/Mat4.h"
#include "Core.h"
#include "LandblockManager.h"
#include "ResourceCache.h"
#include "SimpleModel.h"

#include "graphics/shaders/ModelVertexShader.h"
#include "graphics/shaders/ModelFragmentShader.h"

static ModelRenderData& instantiate(shared_ptr<Destructable>& handle)
{
    auto& model = (SimpleModel&)*handle;
    auto& renderData = model.renderData();

    if(!renderData)
    {
        renderData.reset(new ModelRenderData(model));
    }

    return (ModelRenderData&)*renderData;
}

ModelRenderer::ModelRenderer()
{
    _program.create();
    _program.attach(GL_VERTEX_SHADER, ModelVertexShader);
    _program.attach(GL_FRAGMENT_SHADER, ModelFragmentShader);
    _program.link();
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

        Vec3 landblockOffset(dx * 192.0, dy * 192.0, 0.0);

        for(auto& object : it->second.objects())
        {
            // TODO REMOVE ME
            // 02 models are not yet loaded, so we'll have a null
            if(!object.model)
            {
                continue;
            }

            Mat4 modelOrientMat;
            modelOrientMat.makeRotation(object.orientation);

            Mat4 modelMat;
            modelMat.makeTranslation(landblockOffset + object.position);
            modelMat = modelMat * modelOrientMat;

            auto modelViewMat = viewMat * modelMat;
            auto modelViewProjectionMat = projectionMat * modelViewMat;

            loadMat4ToUniform(modelViewProjectionMat, _program.getUniform("modelViewProjectionMatrix"));

            // TODO FIX UGLY CONST_CAST
            auto& renderData = instantiate(const_cast<shared_ptr<Destructable>&>(object.model));

            renderData.bind();

            glDrawElements(GL_TRIANGLE_STRIP, renderData.indexCount(), GL_UNSIGNED_SHORT, nullptr);
        }
    }
}
