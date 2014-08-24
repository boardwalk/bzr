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

in vec3 fragTexCoord;

out vec4 fragColor;

uniform sampler2D atlas;
uniform sampler1D atlasToc;

void main()
{
    // stpq -- st, normalized lower left; pq, normalized width and height
    vec4 tileExtents = texelFetch(atlasToc, int(fragTexCoord.p + 0.5), 0);

    vec2 tileTexCoord = vec2(1.0) - fract(fragTexCoord.st);
    vec2 atlasTexCoord = tileExtents.st + tileTexCoord * tileExtents.pq;

    vec2 dx = dFdx(fragTexCoord.st * tileExtents.pq);
    vec2 dy = dFdy(fragTexCoord.st * tileExtents.pq);
    //vec2 dx = dFdx(atlasTexCoord);
    //vec2 dy = dFdy(atlasTexCoord);

    fragColor = textureGrad(atlas, atlasTexCoord, dx, dy);
}
