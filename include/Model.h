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
#ifndef BZR_MODEL_H
#define BZR_MODEL_H

#include "physics/AABB.h"
#include "Destructable.h"
#include "Resource.h"
#include "Vertex.h"
#include "TriangleFan.h"

class Model : public ResourceImpl<ResourceType::Model>
{
public:
    Model(uint32_t id, const void* data, size_t size);

    const vector<ResourcePtr>& textures() const;
    const vector<Vertex>& vertices() const;
    const vector<TriangleFan>& triangleFans() const;
    const vector<TriangleFan>& hitTriangleFans() const;
    const AABB& bounds() const;

    // If true, the model has transparent or translucent elements and must be depth sorted before rendering
    bool needsDepthSort() const;

    unique_ptr<Destructable>& renderData();

private:
    vector<ResourcePtr> _textures;
    vector<Vertex> _vertices;
    vector<TriangleFan> _triangleFans;
    vector<TriangleFan> _hitTriangleFans;
    AABB _bounds;
    bool _needsDepthSort;

    unique_ptr<Destructable> _renderData;
};

#endif