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
#ifndef BZR_STRUCTURE_H
#define BZR_STRUCTURE_H

#include "Destructable.h"
#include "Noncopyable.h"
#include "Object.h"

class StructureGeom;

class Structure : Noncopyable
{
public:
    Structure(const void* data, size_t size);
    Structure(Structure&& other);

    const glm::vec3& position() const;
    const glm::quat& rotation() const;
    const vector<ResourcePtr>& textures() const;
    const vector<Object>& objects() const;
    const StructureGeom& geometry() const;
    uint16_t pieceNum() const;

    unique_ptr<Destructable>& renderData();

private:
    glm::vec3 _position;
    glm::quat _rotation;
    vector<ResourcePtr> _textures;
    vector<Object> _objects;
    ResourcePtr _geometry;
    uint16_t _pieceNum;
    unique_ptr<Destructable> _renderData;
};

#endif