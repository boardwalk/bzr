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
#include "physics/Body.h"
#include "physics/LineSegment.h"

static const auto GRAVITY_ACCEL = fp_t(-9.81);
static const auto TERMINAL_VELOCITY = fp_t(-54.0);
static const auto REST_EPSILON = fp_t(0.002);

void Space::step(fp_t dt)
{
    auto numDeadBodies = 0u; // teehee

    for(auto& weakPtr : _bodies)
    {
        auto ptr = weakPtr.lock();

        if(ptr)
        {
            stepBody(*ptr, dt);
        }
        else
        {
            numDeadBodies++;
        }
    }

    if(numDeadBodies * 3 > _bodies.size())
    {
        decltype(_bodies) newBodies;
        newBodies.reserve(_bodies.size() - numDeadBodies);

        for(auto& weakPtr : _bodies)
        {
            if(!weakPtr.expired())
            {
                newBodies.push_back(weakPtr);
            }
        }

        _bodies = move(newBodies);
    }
}

void Space::stepBody(Body& body, fp_t dt)
{
    if(body.flags() & BodyFlags::Static)
    {
        return;
    }

    if(body.flags() & BodyFlags::HasGravity)
    {
        auto velocity = body.velocity();

        velocity.z = velocity.z + GRAVITY_ACCEL * dt;

        if(velocity.z < TERMINAL_VELOCITY)
        {
            velocity.z = TERMINAL_VELOCITY;
        }

        body.setVelocity(velocity);
    }

    if(glm::length(body.velocity()) < REST_EPSILON)
    {
        body.setVelocity(glm::vec3());
    }

    if(body.velocity() == glm::vec3())
    {
        return;
    }

    auto segment = LineSegment();
    segment.begin = body.location().normalize();
    segment.end = segment.begin + body.velocity() * dt;
}
