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
    program_.create();
    program_.attach(GL_VERTEX_SHADER, ModelVertexShader);
    program_.attach(GL_FRAGMENT_SHADER, ModelFragmentShader);
    program_.link();

    program_.use();

    GLuint texLocation = program_.getUniform("tex");
    glUniform1i(texLocation, 0);
}

StructureRenderer::~StructureRenderer()
{
    program_.destroy();
}

void StructureRenderer::render(const glm::mat4& projectionMat, const glm::mat4& viewMat)
{
    program_.use();

    LandcellManager& landcellManager = Core::get().landcellManager();

    glm::vec3 cameraPosition = Core::get().camera().position();
    glUniform4f(program_.getUniform("cameraPosition"), GLfloat(cameraPosition.x), GLfloat(cameraPosition.y), GLfloat(cameraPosition.z), 1.0f);

    for(auto& pair : landcellManager)
    {
        if(!pair.first.isStructure())
        {
            continue;
        }

        int dx = pair.first.x() - landcellManager.center().x();
        int dy = pair.first.y() - landcellManager.center().y();

        glm::vec3 blockPosition{dx * 192.0, dy * 192.0, 0.0};

        const Structure& structure = static_cast<const Structure&>(*pair.second);

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
    glm::mat4 worldMat = glm::translate(glm::mat4{}, position) * glm::mat4_cast(rotation);

    loadMat4ToUniform(worldMat, program_.getUniform("worldMatrix"));
    loadMat4ToUniform(projectionMat * viewMat, program_.getUniform("projectionViewMatrix"));

    if(!structure.renderData())
    {
        structure.renderData().reset(new MeshRenderData(structure));
    }

    MeshRenderData& renderData = static_cast<MeshRenderData&>(*structure.renderData());

    renderData.render();
}
