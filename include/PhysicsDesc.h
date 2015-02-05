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
#ifndef BZR_PHYSICS_DESC
#define BZR_PHYSICS_DESC

#include "LandcellId.h"
#include "Location.h"
#include "Resource.h"

struct PhysicsDesc
{
    struct Relation
    {
        Relation();

        uint32_t objectId;
        uint32_t slot;
    };

    PhysicsDesc();

    uint32_t animFrameId; // ??
    LandcellId landcell;
    Location location;
    ResourcePtr motionTable;
    ResourcePtr soundTable;
    ResourcePtr particleEmitterTable;
    ResourcePtr setup;
    Relation parent;
    vector<Relation> children;
    fp_t scale;
    fp_t friction;
    fp_t elasticity;
    fp_t translucency;
    glm::vec3 velocity;
    glm::vec3 acceleration;
    glm::vec3 angularVelocity;
    ResourcePtr defaultScript;
    fp_t defaultScriptIntensity;
};

void read(BinReader& reader, struct PhysicsDesc& physicsDesc);

#endif