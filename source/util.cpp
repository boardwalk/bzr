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
#include "util.h"
#include "BinReader.h"
#include <glm/gtx/norm.hpp>

void read(BinReader& reader, glm::vec3& v)
{
    v.x = reader.readFloat();
    v.y = reader.readFloat();
    v.z = reader.readFloat();
}

void read(BinReader& reader, glm::quat& q)
{
    q.w = reader.readFloat();
    q.x = reader.readFloat();
    q.y = reader.readFloat();
    q.z = reader.readFloat();

    assert(glm::length2(q) >= 0.99 && glm::length2(q) <= 1.01);
}

void throwSDLError()
{
    throw runtime_error(string("SDL_Init failed: ") + SDL_GetError());
}
