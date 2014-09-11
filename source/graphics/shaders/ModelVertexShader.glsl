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
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoord;

out vec2 fragTexCoord;

uniform vec4 cameraPosition;
uniform mat4 worldMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

const float WORLD_RADIUS = 10000.0;

void main()
{
    vec4 modelPos = vec4(vertexPosition, 1.0);
    vec4 worldPos = worldMatrix * modelPos;
    
    float angle = atan(distance(worldPos.xy, cameraPosition.xy) / WORLD_RADIUS);
    worldPos.z = worldPos.z - WORLD_RADIUS * (1.0 - cos(angle));

    gl_Position = projectionMatrix * viewMatrix * worldPos;
    fragTexCoord = texCoord;
}
