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
#include "Doodad.h"
#include "BinReader.h"
#include "Core.h"
#include "ResourceCache.h"

void Doodad::read(BinReader& reader)
{
    uint32_t modelId = reader.read<uint32_t>();
    resource = Core::get().resourceCache().get(modelId);

    position.x = reader.read<float>();
    position.y = reader.read<float>();
    position.z = reader.read<float>();

    rotation.w = reader.read<float>();
    rotation.x = reader.read<float>();
    rotation.y = reader.read<float>();
    rotation.z = reader.read<float>();
}
