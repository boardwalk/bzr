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
#version 410 core

layout (vertices = 3) out;

in FragmentData
{
    vec2 terrainTexCoord;
    vec4 terrainInfo1;
    vec4 terrainInfo2;
    vec4 terrainInfo3;
    vec4 terrainInfo4;
    vec4 terrainInfo5;
} inData[];

out FragmentData
{
    vec2 terrainTexCoord;
    vec4 terrainInfo1;
    vec4 terrainInfo2;
    vec4 terrainInfo3;
    vec4 terrainInfo4;
    vec4 terrainInfo5;
} outData[];

#include "graphics/shaders/LandCommon.glsl"

float GetTessLevel(float distance0, float distance1)
{
    float avgDistance = (distance0 + distance1) / 2.0;
    return max(80.0 / pow(max(avgDistance, 1.0), 0.6), 1.0);
}

void main()
{
    if(gl_InvocationID == 0)
    {
        float vertexDistance0 = distance(cameraPosition, modelMatrix * gl_in[0].gl_Position);
        float vertexDistance1 = distance(cameraPosition, modelMatrix * gl_in[1].gl_Position);
        float vertexDistance2 = distance(cameraPosition, modelMatrix * gl_in[2].gl_Position);

        gl_TessLevelOuter[0] = GetTessLevel(vertexDistance1, vertexDistance2);
        gl_TessLevelOuter[1] = GetTessLevel(vertexDistance2, vertexDistance0);
        gl_TessLevelOuter[2] = GetTessLevel(vertexDistance0, vertexDistance1);
        gl_TessLevelInner[0] = gl_TessLevelOuter[2];
    }

    outData[gl_InvocationID].terrainTexCoord = inData[gl_InvocationID].terrainTexCoord;
    outData[gl_InvocationID].terrainInfo1 = inData[gl_InvocationID].terrainInfo1;
    outData[gl_InvocationID].terrainInfo2 = inData[gl_InvocationID].terrainInfo2;
    outData[gl_InvocationID].terrainInfo3 = inData[gl_InvocationID].terrainInfo3;
    outData[gl_InvocationID].terrainInfo4 = inData[gl_InvocationID].terrainInfo4;
    outData[gl_InvocationID].terrainInfo5 = inData[gl_InvocationID].terrainInfo5;

    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
}
