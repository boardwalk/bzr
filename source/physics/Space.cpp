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

static bool compareByBeginX(const Body& a, const Body& b)
{
    auto blockDiff = ((int)b.location().landcell.x() - (int)a.location().landcell.x()) * fp_t(192.0);
    return a.location().offset.x + a.bounds().min.x < blockDiff + b.location().offset.x + b.bounds().min.x;
}

bool compareByBeginY(const Body& a, const Body& b)
{
    auto blockDiff = ((int)b.location().landcell.y() - (int)a.location().landcell.y()) * fp_t(192.0);
    return a.location().offset.y + a.bounds().min.y < blockDiff + b.location().offset.y + b.bounds().min.y;
}

bool compareByEndX(const Body& a, const Body& b)
{
    auto blockDiff = ((int)b.location().landcell.x() - (int)a.location().landcell.x()) * fp_t(192.0);
    return a.location().offset.x + a.bounds().max.x < blockDiff + b.location().offset.x + b.bounds().max.x;
}

bool compareByEndY(const Body& a, const Body& b)
{
    auto blockDiff = ((int)b.location().landcell.y() - (int)a.location().landcell.y()) * fp_t(192.0);
    return a.location().offset.y + a.bounds().max.y < blockDiff + b.location().offset.y + b.bounds().max.y;
}

template<class Container, class Compare>
static void insertionSort(Container& container, typename Container::value_type& value, Compare comp)
{
    auto it = container.erase(typename Container::iterator(&value));

    // value < it, compare(value, *it)
    // value >= it, !compare(value, *it)
    // value > it, compare(*it, value)
    // value <= it, !compare(*it, value)
    // FIXME make sure this is correct

    while(comp(value, *it))
    {
        --it;
    }

    while(comp(*it, value))
    {
        ++it;
    }

    container.insert(it, value);
}

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
    resort(body);

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
        return false;
    }

    auto segment = LineSegment();
    segment.begin = body.location().normalize();
    segment.end = segment.begin + body.velocity() * dt;

    return true;
}

void Space::resort(Body& body)
{
    insertionSort(_beginXList, body, compareByBeginX);
    insertionSort(_beginYList, body, compareByBeginY);
    insertionSort(_endXList, body, compareByEndX);
    insertionSort(_endYList, body, compareByEndY);
}
