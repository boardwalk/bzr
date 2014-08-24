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
#ifndef BZR_GRAPHICS_STRUCTURERENDERER_H
#define BZR_GRAPHICS_STRUCTURERENDERER_H

#include "graphics/Program.h"
#include "Noncopyable.h"

struct Mat4;
struct Vec3;
struct Quat;
class Structure;

class StructureRenderer : Noncopyable
{
public:
    StructureRenderer();
    ~StructureRenderer();

    void render(const Mat4& projectionMat, const Mat4& viewMat);

private:
    void renderStructure(Structure& structure, const Mat4& projectionMat, const Mat4& view, const Vec3& position, const Quat& rotation);

    Program _program;
};

#endif