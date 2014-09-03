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
#include "graphics/SkyRenderer.h"
#include "graphics/SkyModel.h"
#include "graphics/util.h"
#include "Camera.h"
#include "Core.h"

#include "graphics/shaders/SkyVertexShader.h"
#include "graphics/shaders/SkyFragmentShader.h"

static const int CUBE_SIZE = 256;

SkyRenderer::SkyRenderer()
{
    initProgram();
    initGeometry();
    initTexture();

    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
}

SkyRenderer::~SkyRenderer()
{
    _program.destroy();
    glDeleteVertexArrays(1, &_vertexArray);
    glDeleteBuffers(1, &_vertexBuffer);
    glDeleteTextures(1, &_texture);
}

void SkyRenderer::render()
{
    _program.use();

    auto rotationMat = glm::mat4_cast(glm::conjugate(Core::get().camera().rotationQuat()));

    loadMat4ToUniform(rotationMat, _program.getUniform("rotationMat"));

    glBindVertexArray(_vertexArray);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, _texture);

    glDisable(GL_DEPTH_TEST);

    glDrawArrays(GL_TRIANGLES, 0, _vertexCount);

    glEnable(GL_DEPTH_TEST);
}

void SkyRenderer::initProgram()
{
    _program.create();
    _program.attach(GL_VERTEX_SHADER, SkyVertexShader);
    _program.attach(GL_FRAGMENT_SHADER, SkyFragmentShader);
    _program.link();
}

void SkyRenderer::initGeometry()
{
    static float VERTEX_DATA[] =
    {
        -1.0, -1.0,
         1.0, -1.0,
        -1.0,  1.0,
         1.0,  1.0,
        -1.0,  1.0,
         1.0, -1.0
    };

    static const int COMPONENTS_PER_VERTEX = 2;

    _vertexCount = sizeof(VERTEX_DATA) / sizeof(VERTEX_DATA[0]) / COMPONENTS_PER_VERTEX;

    glGenVertexArrays(1, &_vertexArray);
    glBindVertexArray(_vertexArray);

    glGenBuffers(1, &_vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, _vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(VERTEX_DATA), VERTEX_DATA, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * COMPONENTS_PER_VERTEX, nullptr);

    glEnableVertexAttribArray(0);
}

void SkyRenderer::initTexture()
{
    static const GLenum FACES[] =
    {
        GL_TEXTURE_CUBE_MAP_POSITIVE_X,
        GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
        GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
        GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
        GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
        GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
    };

    static const int NUM_FACES = sizeof(FACES) / sizeof(FACES[0]);

    glGenTextures(1, &_texture);
    glBindTexture(GL_TEXTURE_CUBE_MAP, _texture);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    SkyModel model;

    SkyModel::Params params;
    params.dt = fp_t(180.0);
    params.tm = fp_t(0.7);
    params.lng = fp_t(0.0);
    params.lat = fp_t(0.0);
    params.tu = fp_t(5.0);
    model.prepare(params);

    vector<uint8_t> data(CUBE_SIZE * CUBE_SIZE * 3);

    for(auto face = 0; face < NUM_FACES; face++)
    {
        for(auto j = 0; j < CUBE_SIZE; j++)
        {
            for(auto i = 0; i < CUBE_SIZE; i++)
            {
                // scale to cube face
                auto fi = fp_t(i) / fp_t(CUBE_SIZE - 1) * 2.0 - 1.0;
                auto fj = fp_t(j) / fp_t(CUBE_SIZE - 1) * 2.0 - 1.0;

                // find point on the cube we're mapping
                glm::vec3 cp;

                switch(face)
                {
                    case 0: cp = glm::vec3( 1.0,  -fj,  -fi); break; // +X
                    case 1: cp = glm::vec3(-1.0,  -fj,   fi); break; // -X
                    case 2: cp = glm::vec3(  fi,  1.0,   fj); break; // +Y
                    case 3: cp = glm::vec3(  fi, -1.0,  -fj); break; // -Y
                    case 4: cp = glm::vec3(  fi,  -fj,  1.0); break; // +Z
                    case 5: cp = glm::vec3( -fi,  -fj, -1.0); break; // -Z
                }

                // map cube to sphere
                // http://mathproofs.blogspot.com/2005/07/mapping-cube-to-sphere.html
                glm::vec3 sp;

                sp.x = cp.x * glm::sqrt(fp_t(1.0) - cp.y * cp.y / fp_t(2.0) - cp.z * cp.z / fp_t(2.0) + cp.y * cp.y * cp.z * cp.z / fp_t(3.0));
                sp.y = cp.y * glm::sqrt(fp_t(1.0) - cp.z * cp.z / fp_t(2.0) - cp.x * cp.x / fp_t(2.0) + cp.z * cp.z * cp.x * cp.x / fp_t(3.0));
                sp.z = cp.z * glm::sqrt(fp_t(1.0) - cp.x * cp.x / fp_t(2.0) - cp.y * cp.y / fp_t(2.0) + cp.x * cp.x * cp.y * cp.y / fp_t(3.0));

                // convert cartesian to spherical
                // http://en.wikipedia.org/wiki/Spherical_coordinate_system#Cartesian_coordinates
                // phi is ccw from -y, not ccw from +x
                auto theta = acos(sp.z / sqrt(sp.x * sp.x + sp.y * sp.y + sp.z * sp.z));
                auto phi = atan2(sp.x, -sp.y);

                // pull edge below landblock edge
                theta *= fp_t(0.9);

                // compute and store color
                auto color = model.getColor(theta, phi);
                data[(i + j * CUBE_SIZE) * 3] = (uint8_t)(color.x * 0xFF);
                data[(i + j * CUBE_SIZE) * 3 + 1] = (uint8_t)(color.y * 0xFF);
                data[(i + j * CUBE_SIZE) * 3 + 2] = (uint8_t)(color.z * 0xFF);
            }
        }

        glTexImage2D(FACES[face], 0, GL_RGB8, CUBE_SIZE, CUBE_SIZE, 0, GL_RGB, GL_UNSIGNED_BYTE, data.data());
    }

    _sunVector.x = sin(model.thetaSun()) * sin(model.phiSun());
    _sunVector.y = -sin(model.thetaSun()) * cos(model.phiSun());
    _sunVector.z = cos(model.thetaSun());
}

const glm::vec3& SkyRenderer::sunVector() const
{
    return _sunVector;
}

