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
#include "graphics/util.h"
#include "math/Mat3.h"
#include "math/Mat4.h"

void loadMat3ToUniform(const Mat3& mat, GLint location)
{
    GLfloat m[9];

    for(auto i = 0; i < 9; i++)
    {
        m[i] = GLfloat(mat.m[i]);
    }

    glUniformMatrix3fv(location, 1, GL_FALSE, m);
}

void loadMat4ToUniform(const Mat4& mat, GLint location)
{
    GLfloat m[16];

    for(auto i = 0; i < 16; i++)
    {
       m[i] = GLfloat(mat.m[i]);
    }

    glUniformMatrix4fv(location, 1, GL_FALSE, m);
}
