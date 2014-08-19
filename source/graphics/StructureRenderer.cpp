#include "graphics/StructureRenderer.h"
#include "graphics/MeshRenderData.h"
#include "graphics/Renderer.h"
#include "graphics/TextureAtlas.h"
#include "graphics/util.h"
#include "math/Mat4.h"
#include "Core.h"
#include "LandblockManager.h"

// We're sharing shaders with the model renderer for now
#include "graphics/shaders/ModelVertexShader.h"
#include "graphics/shaders/ModelFragmentShader.h"

StructureRenderer::StructureRenderer()
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

StructureRenderer::~StructureRenderer()
{
    _program.destroy();
}

void StructureRenderer::render(const Mat4& projectionMat, const Mat4& viewMat)
{
    _program.use();

    Core::get().renderer().textureAtlas().bind();

    auto& landblockManager = Core::get().landblockManager();

    for(auto& pair : landblockManager)
    {
        auto dx = pair.first.x() - landblockManager.center().x();
        auto dy = pair.first.y() - landblockManager.center().y();

        auto landblockPosition = Vec3(dx * 192.0, dy * 192.0, 0.0);

        for(auto& structure : pair.second.structures())
        {
            renderStructure(const_cast<Structure&>(structure), projectionMat, viewMat, landblockPosition + structure.position(), structure.rotation());
        }
    }
}

void StructureRenderer::renderStructure(Structure& structure, const Mat4& projectionMat, const Mat4& viewMat, const Vec3& position, const Quat& rotation)
{
    Mat4 modelRotationMat;
    modelRotationMat.makeRotation(rotation);

    Mat4 modelTranslationMat;
    modelTranslationMat.makeTranslation(position);

    auto modelMat = modelTranslationMat * modelRotationMat;
    auto modelViewProjectionMat = projectionMat * viewMat * modelMat;

    loadMat4ToUniform(modelViewProjectionMat, _program.getUniform("modelViewProjectionMatrix"));

    if(!structure.renderData())
    {
        structure.renderData().reset(new MeshRenderData(structure));
    }

    auto& renderData = (MeshRenderData&)*structure.renderData();

    renderData.bind();

    glDrawElements(GL_TRIANGLE_FAN, renderData.indexCount(), GL_UNSIGNED_SHORT, nullptr);
}
