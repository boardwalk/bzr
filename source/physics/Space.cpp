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
#include "physics/Space.h"
#include "physics/LineSegment.h"

static const auto GRAVITY_ACCEL = fp_t(-9.81);
static const auto TERMINAL_VELOCITY = fp_t(-54.0);
static const auto REST_EPSILON = fp_t(0.002);

void Space::step(fp_t dt)
{
    for(auto it = _activeList.begin(); it != _activeList.end(); ++it)
    {
        if(step(dt, *it))
        {
            ++it;
        }
        else
        {
            it = _activeList.erase(it);
        }
    }
}

void Space::insert(Body& body)
{
    _beginXList.push_back(body);
    _beginYList.push_back(body);
    _endXList.push_back(body);
    _endYList.push_back(body);
    // TODO keep lists sorted

    if(!(body.flags() & BodyFlags::Static))
    {
        _activeList.push_back(body);
    }
}

bool Space::step(fp_t dt, Body& body)
{
    if(body.flags() & BodyFlags::HasGravity)
    {
        auto velocity = body.velocity();

        velocity.z = max(velocity.z + GRAVITY_ACCEL * dt, TERMINAL_VELOCITY);

        body.setVelocity(velocity);
    }

    if(glm::length(body.velocity()) < REST_EPSILON)
    {
        body.setVelocity(glm::vec3());
    }

    if(body.velocity() == glm::vec3())
    {
        return false;
    }

    auto segment = LineSegment();
    segment.begin = body.location().normalize();
    segment.end = segment.begin + body.velocity() * dt;

    return true;
}
