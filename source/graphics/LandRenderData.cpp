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
#include "graphics/LandRenderData.h"
#include "graphics/Program.h"
#include "Land.h"

LandRenderData::LandRenderData(const Land& land)
{
    initGeometry(land);
    initNormalTexture(land);
}

LandRenderData::~LandRenderData()
{
    glDeleteVertexArrays(1, &vertexArray_);
    glDeleteBuffers(1, &vertexBuffer_);
    glDeleteTextures(1, &normalTexture_);
}

void LandRenderData::render()
{
    glBindVertexArray(vertexArray_);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, normalTexture_);

    glDrawArrays(GL_TRIANGLES, 0, vertexCount_);
}

static void pushRotatedCoord(vector<uint8_t>& vertexData, fp_t s, fp_t t, int rotations, uint8_t scale)
{
    fp_t cosine = glm::cos(pi() / fp_t(180.0) * fp_t(90.0) * rotations);
    fp_t sine = glm::sin(pi() / fp_t(180.0) * fp_t(90.0) * rotations);

    fp_t ns = (s - fp_t(0.5)) * cosine - (t - fp_t(0.5)) * sine + fp_t(0.5);
    fp_t nt = (s - fp_t(0.5)) * sine + (t - fp_t(0.5)) * cosine + fp_t(0.5);

    vertexData.push_back(static_cast<uint8_t>(ns + fp_t(0.5)) * scale);
    vertexData.push_back(static_cast<uint8_t>(nt + fp_t(0.5)) * scale);
}

void LandRenderData::initGeometry(const Land& land)
{
    const Land::Data& data = land.data();

    vector<uint8_t> vertexData;

    for(uint8_t y = 0; y < Land::kGridSize - 1; y++)
    {
        for(uint8_t x = 0; x < Land::kGridSize - 1; x++)
        {
#define T(dx, dy) static_cast<uint8_t>((data.styles[x + (dx)][y + (dy)] >> 2) & 0x1F)
            uint8_t terrain[]
            {
                T(0, 0), T(1, 0), T(1, 1), T(0, 1)
            };
#undef T

#define R(dx, dy) static_cast<uint8_t>(data.styles[x + (dx)][y + (dy)] & 0x3)
            uint8_t road[]
            {
                R(0, 0), R(1, 0), R(1, 1), R(0, 1)
            };
#undef R

            uint32_t terrainDone = 0;

            vector<uint8_t> textures;
            vector<uint8_t> blendTextures;
            vector<uint8_t> rotations;

            for(int i = 0; i < 4; i++)
            {
                if(terrainDone & (1 << terrain[i]))
                {
                    continue;
                }

                terrainDone |= (1 << terrain[i]);

                uint8_t bitfield = 0;

                for(int j = 0; j < 4; j++)
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

                for(int j = 0; j < 4; j++)
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

            if(land.isSplitNESW(x, y))
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

    static const int kComponentsPerVertex = 25;

    vertexCount_ = static_cast<GLsizei>(vertexData.size()) / kComponentsPerVertex;

    glGenVertexArrays(1, &vertexArray_);
    glBindVertexArray(vertexArray_);

    glGenBuffers(1, &vertexBuffer_);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer_);
    glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(uint8_t), vertexData.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_UNSIGNED_BYTE, GL_FALSE, kComponentsPerVertex * sizeof(uint8_t), nullptr);
    glVertexAttribPointer(1, 2, GL_UNSIGNED_BYTE, GL_FALSE, kComponentsPerVertex * sizeof(uint8_t), reinterpret_cast<GLvoid*>(sizeof(uint8_t) * 3));
    glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_FALSE, kComponentsPerVertex * sizeof(uint8_t), reinterpret_cast<GLvoid*>(sizeof(uint8_t) * 5));
    glVertexAttribPointer(3, 4, GL_UNSIGNED_BYTE, GL_FALSE, kComponentsPerVertex * sizeof(uint8_t), reinterpret_cast<GLvoid*>(sizeof(uint8_t) * 9));
    glVertexAttribPointer(4, 4, GL_UNSIGNED_BYTE, GL_FALSE, kComponentsPerVertex * sizeof(uint8_t), reinterpret_cast<GLvoid*>(sizeof(uint8_t) * 13));
    glVertexAttribPointer(5, 4, GL_UNSIGNED_BYTE, GL_FALSE, kComponentsPerVertex * sizeof(uint8_t), reinterpret_cast<GLvoid*>(sizeof(uint8_t) * 17));
    glVertexAttribPointer(6, 4, GL_UNSIGNED_BYTE, GL_FALSE, kComponentsPerVertex * sizeof(uint8_t), reinterpret_cast<GLvoid*>(sizeof(uint8_t) * 21));

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);
    glEnableVertexAttribArray(4);
    glEnableVertexAttribArray(5);
    glEnableVertexAttribArray(6);
}

void LandRenderData::initNormalTexture(const Land& land)
{
    glGenTextures(1, &normalTexture_);
    glBindTexture(GL_TEXTURE_2D, normalTexture_);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, Land::kOffsetMapSize, Land::kOffsetMapSize, 0, GL_RGB, GL_UNSIGNED_BYTE, land.normalMap());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); // default is GL_NEAREST_MIPMAP_LINEAR
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}
