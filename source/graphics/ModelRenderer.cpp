#include "graphics/ModelRenderer.h"
#include "graphics/ModelRenderData.h"
#include "graphics/util.h"
#include "math/Mat4.h"
#include "Core.h"
#include "LandblockManager.h"
#include "Model.h"
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

        Vec3 landblockOffset(dx * 192.0, dy * 192.0, 0.0);

        for(auto& object : it->second.objects())
        {
            // FIXME
            // We don't support rendering 02 models yet
            if(object.model->resourceType() != Resource::Model)
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
            auto& model = const_cast<ResourcePtr&>(object.model)->cast<Model>();

            if(!model.renderData())
            {
                model.renderData().reset(new ModelRenderData(model));
            }

            auto& renderData = (ModelRenderData&)*model.renderData();

            renderData.bind();

            glDrawElements(GL_TRIANGLE_FAN, renderData.indexCount(), GL_UNSIGNED_SHORT, nullptr);
        }
    }
}
