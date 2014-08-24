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

layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec2 terrainTexCoord;
layout(location = 2) in vec4 terrainInfo1;
layout(location = 3) in vec4 terrainInfo2;
layout(location = 4) in vec4 terrainInfo3;
layout(location = 5) in vec4 terrainInfo4;
layout(location = 6) in vec4 terrainInfo5;

out FragmentData
{
    vec3 position;
    vec2 normalTexCoord;
    vec2 terrainTexCoord;
    vec4 terrainInfo1;
    vec4 terrainInfo2;
    vec4 terrainInfo3;
    vec4 terrainInfo4;
    vec4 terrainInfo5;
} fragData;

#include "graphics/shaders/LandCommon.glsl"

const float WORLD_RADIUS = 10000.0;

void main()
{
    vec4 modelPos = vec4(vertexPosition, 1.0) * vec4(24.0, 24.0, 2.0, 1.0);

    vec4 worldPos = worldMatrix * modelPos;
    float angle = atan(distance(worldPos.xy, cameraPosition.xy) / WORLD_RADIUS);
    modelPos.z = modelPos.z - WORLD_RADIUS * (1.0 - cos(angle));

    gl_Position = worldViewProjectionMatrix * modelPos;

    fragData.position = (worldViewMatrix * modelPos).xyz;
    fragData.normalTexCoord = modelPos.xy / 192.0;
    fragData.terrainTexCoord = terrainTexCoord;
    fragData.terrainInfo1 = terrainInfo1;
    fragData.terrainInfo2 = terrainInfo2;
    fragData.terrainInfo3 = terrainInfo3;
    fragData.terrainInfo4 = terrainInfo4;
    fragData.terrainInfo5 = terrainInfo5;
}
