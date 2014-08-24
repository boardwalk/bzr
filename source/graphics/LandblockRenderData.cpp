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
#include "graphics/LandblockRenderData.h"
#include "graphics/Program.h"
#include "Landblock.h"

LandblockRenderData::LandblockRenderData(const Landblock& landblock)
{
    initGeometry(landblock);
    initOffsetTexture(landblock);
    initNormalTexture(landblock);
}

LandblockRenderData::~LandblockRenderData()
{
    glDeleteVertexArrays(1, &_vertexArray);
    glDeleteBuffers(1, &_vertexBuffer);
    glDeleteTextures(1, &_offsetTexture);
    glDeleteTextures(1, &_normalTexture);
}

void LandblockRenderData::bind(Program& program)
{
    glBindVertexArray(_vertexArray);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, _offsetTexture);

    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, _normalTexture);

    auto offsetBaseLoc = program.getUniform("offsetBase");
    glUniform1f(offsetBaseLoc, _offsetBase);

    auto offsetScaleLoc = program.getUniform("offsetScale");
    glUniform1f(offsetScaleLoc, _offsetScale);
}

GLsizei LandblockRenderData::vertexCount() const
{
	return _vertexCount;
}

static void pushRotatedCoord(vector<uint8_t>& vertexData, double s, double t, int rotations, uint8_t scale)
{
    auto cosine = cos(PI / 180.0 * 90.0 * rotations);
    auto sine = sin(PI / 180.0 * 90.0 * rotations);

    auto ns = (s - 0.5) * cosine - (t - 0.5) * sine + 0.5;
    auto nt = (s - 0.5) * sine + (t - 0.5) * cosine + 0.5;

    vertexData.push_back(uint8_t(ns + 0.5) * scale);
    vertexData.push_back(uint8_t(nt + 0.5) * scale);
}

void LandblockRenderData::initGeometry(const Landblock& landblock)
{
    auto& data = landblock.rawData();

    vector<uint8_t> vertexData;

    for(uint8_t y = 0; y < Landblock::GRID_SIZE - 1; y++)
    {
        for(uint8_t x = 0; x < Landblock::GRID_SIZE - 1; x++)
        {
            uint8_t terrain[4];

#define T(dx, dy) (data.styles[x + (dx)][y + (dy)] >> 2) & 0x1F
            terrain[0] = T(0, 0);
            terrain[1] = T(1, 0);
            terrain[2] = T(1, 1);
            terrain[3] = T(0, 1);
#undef T

            uint8_t road[4];

#define R(dx, dy) data.styles[x + (dx)][y + (dy)] & 0x3
            road[0] = R(0, 0);
            road[1] = R(1, 0);
            road[2] = R(1, 1);
            road[3] = R(0, 1);
#undef R

            uint32_t terrainDone = 0;

            vector<uint8_t> textures;
            vector<uint8_t> blendTextures;
            vector<uint8_t> rotations;

            for(auto i = 0; i < 4; i++)
            {
                if(terrainDone & (1 << terrain[i]))
                {
                    continue;
                }

                terrainDone |= (1 << terrain[i]);

                uint8_t bitfield = 0;

                for(auto j = 0; j < 4; j++)
                {
                    if(terrain[j] == terrain[i])
                    {
                        bitfield |= (1 << j);
                    }
                }

                // number of 90 degree ccw rotations
                uint8_t rotationCount = 0;
                uint8_t blendTex = 0xFF;

                for(;;)
                {
                    switch(bitfield)
                    {
                        case 0x1: // 0001
                            blendTex = 8;
                            break;
                        case 0x9: // 1001
                            blendTex = 2;
                            break;
                        case 0x5: // 0101
                            blendTex = 0; // TODO
                            break;
                        case 0xE: // 1110
                            blendTex = 0x80 + 0x8;
                            break;
                        case 0xF: // 1111
                            blendTex = 1;
                            break;
                    }

                    if(blendTex != 0xFF)
                    {
                        break;
                    }

                    bitfield = ((bitfield << 1) | (bitfield >> 3)) & 0xF;
                    rotationCount++;
                }

                textures.push_back(terrain[i]);
                blendTextures.push_back(blendTex);
                rotations.push_back(rotationCount);
            }

            while(textures.size() < 4)
            {
                textures.push_back(0);
                blendTextures.push_back(0);
                rotations.push_back(0);
            }

            uint8_t roadScale = 3;

            {
                uint8_t bitfield = 0;

                for(auto j = 0; j < 4; j++)
                {
                    if(road[j])
                    {
                        bitfield |= (1 << j);
                    }
                }

                uint8_t rotationCount = 0;
                uint8_t blendTex = 0xFF;

                for(;;)
                {
                    switch(bitfield)
                    {
                        case 0x0: // 0000
                            blendTex = 0;
                            break;
                        case 0x1: // 0001
                            blendTex = 5;
                            break;
                        case 0x9: // 1001
                            blendTex = 2;
                            break;
                        case 0x5: // 0101
                            blendTex =  0xA;
                            roadScale = 1;
                            break;
                        case 0xE: // 1110
                            blendTex = 0x80 + 5;
                            break;
                        case 0xF: // 1111
                            blendTex = 1;
                            break;
                    }

                    if(blendTex != 0xFF)
                    {
                        break;
                    }

                    bitfield = ((bitfield << 1) | (bitfield >> 3)) & 0xF;
                    rotationCount++;
                }

                textures.push_back(0x20);
                blendTextures.push_back(blendTex);
                rotations.push_back(rotationCount);
            }

            // See LandVertexShader.glsl to see what these are
#define V(dx, dy) \
    vertexData.push_back(x + (dx)); \
    vertexData.push_back(y + (dy)); \
    vertexData.push_back(data.heights[x + (dx)][y + (dy)]); \
    vertexData.push_back(dx); \
    vertexData.push_back(dy); \
    pushRotatedCoord(vertexData, dx, dy, rotations[0], 1); \
    vertexData.push_back(blendTextures[0]); \
    vertexData.push_back(textures[0]); \
    pushRotatedCoord(vertexData, dx, dy, rotations[1], 1); \
    vertexData.push_back(blendTextures[1]); \
    vertexData.push_back(textures[1]); \
    pushRotatedCoord(vertexData, dx, dy, rotations[2], 1); \
    vertexData.push_back(blendTextures[2]); \
    vertexData.push_back(textures[2]); \
    pushRotatedCoord(vertexData, dx, dy, rotations[3], 1); \
    vertexData.push_back(blendTextures[3]); \
    vertexData.push_back(textures[3]); \
    pushRotatedCoord(vertexData, dx, dy, rotations[4], roadScale); \
    vertexData.push_back(blendTextures[4]); \
    vertexData.push_back(textures[4]);

            if(landblock.isSplitNESW(x, y))
            {
                // lower right triangle
                V(0, 0) V(1, 0) V(1, 1)

                // top left triangle
                V(1, 1) V(0, 1) V(0, 0)
            }
            else
            {
                // lower left triangle
                V(0, 0) V(1, 0) V(0, 1)

                // top right triangle
                V(1, 0) V(1, 1) V(0, 1)
            }
#undef V
        }
    }

    static const int COMPONENTS_PER_VERTEX = 25;

    _vertexCount = GLsizei(vertexData.size()) / COMPONENTS_PER_VERTEX;

    glGenVertexArrays(1, &_vertexArray);
    glBindVertexArray(_vertexArray);

    glGenBuffers(1, &_vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, _vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(uint8_t), vertexData.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_UNSIGNED_BYTE, GL_FALSE, COMPONENTS_PER_VERTEX * sizeof(uint8_t), nullptr);
    glVertexAttribPointer(1, 2, GL_UNSIGNED_BYTE, GL_FALSE, COMPONENTS_PER_VERTEX * sizeof(uint8_t), (GLvoid*)(sizeof(uint8_t) * 3));
    glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_FALSE, COMPONENTS_PER_VERTEX * sizeof(uint8_t), (GLvoid*)(sizeof(uint8_t) * 5));
    glVertexAttribPointer(3, 4, GL_UNSIGNED_BYTE, GL_FALSE, COMPONENTS_PER_VERTEX * sizeof(uint8_t), (GLvoid*)(sizeof(uint8_t) * 9));
    glVertexAttribPointer(4, 4, GL_UNSIGNED_BYTE, GL_FALSE, COMPONENTS_PER_VERTEX * sizeof(uint8_t), (GLvoid*)(sizeof(uint8_t) * 13));
    glVertexAttribPointer(5, 4, GL_UNSIGNED_BYTE, GL_FALSE, COMPONENTS_PER_VERTEX * sizeof(uint8_t), (GLvoid*)(sizeof(uint8_t) * 17));
    glVertexAttribPointer(6, 4, GL_UNSIGNED_BYTE, GL_FALSE, COMPONENTS_PER_VERTEX * sizeof(uint8_t), (GLvoid*)(sizeof(uint8_t) * 21));

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);
    glEnableVertexAttribArray(4);
    glEnableVertexAttribArray(5);
    glEnableVertexAttribArray(6);
}

void LandblockRenderData::initOffsetTexture(const Landblock& landblock)
{
    glGenTextures(1, &_offsetTexture);
    glBindTexture(GL_TEXTURE_2D, _offsetTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R16, Landblock::OFFSET_MAP_SIZE, Landblock::OFFSET_MAP_SIZE, 0, GL_RED, GL_UNSIGNED_SHORT, landblock.offsetMap());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); // default is GL_NEAREST_MIPMAP_LINEAR
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    _offsetBase = GLfloat(landblock.offsetMapBase());
    _offsetScale = GLfloat(landblock.offsetMapScale());
}

void LandblockRenderData::initNormalTexture(const Landblock& landblock)
{
    glGenTextures(1, &_normalTexture);
    glBindTexture(GL_TEXTURE_2D, _normalTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, Landblock::OFFSET_MAP_SIZE, Landblock::OFFSET_MAP_SIZE, 0, GL_RGB, GL_UNSIGNED_BYTE, landblock.normalMap());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); // default is GL_NEAREST_MIPMAP_LINEAR
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

