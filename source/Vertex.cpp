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
#include "Vertex.h"
#include "BinReader.h"

void Vertex::read(BinReader& reader)
{
    uint16_t numTexCoords = reader.read<uint16_t>();
    texCoords.resize(numTexCoords);

    position.x = reader.read<float>();
    position.y = reader.read<float>();
    position.z = reader.read<float>();

    normal.x = reader.read<float>();
    normal.y = reader.read<float>();
    normal.z = reader.read<float>();

    for(glm::vec2& texCoord : texCoords)
    {
        texCoord.x = reader.read<float>();
        texCoord.y = reader.read<float>();
    }
}
