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

const float PI = 3.14159265359;

// textures
uniform sampler2DArray terrainTex;
uniform sampler2DArray blendTex;
uniform sampler2D normalTex;

// lighting parameters
uniform vec4 cameraPosition;
uniform vec3 lightPosition;
uniform vec3 lightIntensity;
uniform vec3 Kd;
uniform vec3 Ka;
uniform vec3 Ks;
uniform float shininess;

uniform mat3 normalMatrix;

// model space to world space
uniform mat4 worldMatrix;

// model space to view space
uniform mat4 worldViewMatrix;

// model space to clip space
uniform mat4 worldViewProjectionMatrix;
