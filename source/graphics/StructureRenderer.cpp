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
#include "graphics/StructureRenderer.h"
#include "graphics/MeshRenderData.h"
#include "graphics/Renderer.h"
#include "graphics/util.h"
#include "Camera.h"
#include "Core.h"
#include "LandcellManager.h"
#include "Landcell.h"
#include "Structure.h"
#include <glm/gtc/matrix_transform.hpp>

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


    auto texLocation = _program.getUniform("tex");
    glUniform1i(texLocation, 0);
}

StructureRenderer::~StructureRenderer()
{
    _program.destroy();
}

void StructureRenderer::render(const glm::mat4& projectionMat, const glm::mat4& viewMat)
{
    _program.use();

    auto& landcellManager = Core::get().landcellManager();

    auto cameraPosition = Core::get().camera().position();
    glUniform4f(_program.getUniform("cameraPosition"), GLfloat(cameraPosition.x), GLfloat(cameraPosition.y), GLfloat(cameraPosition.z), 1.0f);

    for(auto& pair : landcellManager)
    {
        if(!pair.first.isStructure())
        {
            continue;
        }

        auto dx = pair.first.x() - landcellManager.center().x();
        auto dy = pair.first.y() - landcellManager.center().y();

        auto blockPosition = glm::vec3(dx * 192.0, dy * 192.0, 0.0);

        auto& structure = static_cast<const Structure&>(*pair.second);

        renderStructure(structure, projectionMat, viewMat, blockPosition + structure.position(), structure.rotation());
    }
}

void StructureRenderer::renderStructure(
    const Structure& structure,
    const glm::mat4& projectionMat,
    const glm::mat4& viewMat,
    const glm::vec3& position,
    const glm::quat& rotation)
{
    auto worldMat = glm::translate(glm::mat4(), position) * glm::mat4_cast(rotation);
    auto worldViewProjectionMat = projectionMat * viewMat * worldMat;

    loadMat4ToUniform(worldMat, _program.getUniform("worldMatrix"));
    loadMat4ToUniform(worldViewProjectionMat, _program.getUniform("worldViewProjectionMatrix"));

    if(!structure.renderData())
    {
        structure.renderData().reset(new MeshRenderData(structure));
    }

    auto& renderData = (MeshRenderData&)*structure.renderData();

    renderData.render();
}
