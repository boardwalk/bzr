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

layout(location = 0) in vec2 vertexPosition;

out vec3 texCoord;

uniform mat4 rotationMat;

void main()
{
    gl_Position = vec4(vertexPosition, 0.5, 1.0);
    texCoord = (rotationMat * vec4(vertexPosition.x, vertexPosition.y, -1.0, 1.0)).xyz;
}
