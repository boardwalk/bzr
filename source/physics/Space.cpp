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
#include "Landblock.h"
#include <algorithm>

static const auto GRAVITY_ACCEL = fp_t(-9.81);
static const auto TERMINAL_VELOCITY = fp_t(-54.0);
static const auto REST_EPSILON = fp_t(0.002);

template<int Axis>
struct AxisTraits;

template<>
struct AxisTraits<0>
{
    static const int BeginHook = BodyHooks::BeginX;
    static const int EndHook = BodyHooks::EndX;
};

template<>
struct AxisTraits<1>
{
    static const int BeginHook = BodyHooks::BeginY;
    static const int EndHook = BodyHooks::EndY;
};

template<int Axis>
fp_t getBoundValue(ilist<Body>::iterator it)
{
    if(it.on_hook<AxisTraits<Axis>::BeginHook>())
    {
        return it->bounds().min[Axis];
    }

    if(it.on_hook<AxisTraits<Axis>::EndHook>())
    {
        return it->bounds().max[Axis];
    }

    assert(!"Iterator for axis associated with unknown hook");
    return 0.0;
}

template<int Axis>
bool compare(ilist<Body>::iterator a, ilist<Body>::iterator b)
{
    fp_t aValue = a->location().offset[Axis] + getBoundValue<Axis>(a);
    fp_t bValue = b->location().offset[Axis] + getBoundValue<Axis>(b);
    int blockDiff = static_cast<int>(b->location().landcell[Axis]) - static_cast<int>(a->location().landcell[Axis]);
    return aValue < bValue + blockDiff * Landblock::LANDBLOCK_SIZE;
}

template<int Axis, class OverlapMap>
static void insertionSort(ilist<Body>& container, ilist<Body>::iterator bodyIt, OverlapMap& overlapMap)
{
    // a < b, compare(a, b)
    // a >= b, !compare(a, b)
    // a > b, compare(b, a)
    // a <= b, !compare(b, a)
    // FIXME make sure this is correct

    auto nextIt = container.erase(bodyIt);

    while(nextIt != container.end() && !compare<Axis>(nextIt, bodyIt)) // bodyIt <= nextIt
    {
        pair<Body*, Body*> key(&*nextIt, &*bodyIt);

        if(key.first > key.second)
        {
            swap(key.first, key.second);
        }

        // (bodyIt) (nextIt before)< (nextIt after)
        // (nextIt before)  (bodyIt) (nextIt after)<
        // We're moving bodyIt *past* nextIt (before we increment it)

        // if bodyIt == begin hook and nextIt == begin hook
        //   nothing happens
        // if bodyIt == end hook and nextIt == begin hook
        //   set to true!
        // if bodyIt == begin hook and nextIt == end hook
        //   set to false!
        // if bodyIt == end hook and nextIt == end hook
        //   nothing happens

        if(bodyIt.on_hook<AxisTraits<Axis>::BeginHook>() && nextIt.on_hook<AxisTraits<Axis>::EndHook>())
        {
            overlapMap[key][Axis] = false;
        }
        else if(bodyIt.on_hook<AxisTraits<Axis>::EndHook>() && nextIt.on_hook<AxisTraits<Axis>::BeginHook>())
        {
            overlapMap[key][Axis] = true;
        }

        ++nextIt;
    }

    // now bodyIt > nextIt, one past bodyIt <= nextIt
    --nextIt;
    // now bodyIt <= nextIt

    container.insert(nextIt, bodyIt);
}

Space::Space()
{}

void Space::step(fp_t dt)
{
    // loop through active list and step bodies
    for(auto it = _activeList.begin(); it != _activeList.end(); /**/)
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

    // scan our overlap map and process possible collisions
    for(auto it = _overlapMap.begin(); it != _overlapMap.end(); /**/)
    {
        if(it->second[0] && it->second[1])
        {
            collide(*it->first.first, *it->first.second);
            ++it;
        }
        else if(!it->second[0] && !it->second[1])
        {
            it = _overlapMap.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

void Space::insert(Body& body)
{
    _xAxisList.push_back(_xAxisList.iterator_for<BodyHooks::BeginX>(body));
    _xAxisList.push_back(_xAxisList.iterator_for<BodyHooks::EndX>(body));
    _yAxisList.push_back(_yAxisList.iterator_for<BodyHooks::BeginY>(body));
    _yAxisList.push_back(_yAxisList.iterator_for<BodyHooks::EndY>(body));

    if(!(body.flags() & BodyFlags::Static))
    {
        _activeList.push_back(_activeList.iterator_for<BodyHooks::Active>(body));
    }

    resort(body);
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

    auto location = body.location();
    location.offset += body.velocity() * dt;
    body.setLocation(location);

    resort(body);

    return true;
}

void Space::resort(Body& body)
{
    insertionSort<0>(_xAxisList, _xAxisList.iterator_for<BodyHooks::BeginX>(body), _overlapMap);
    insertionSort<0>(_xAxisList, _xAxisList.iterator_for<BodyHooks::EndX>(body), _overlapMap);
    insertionSort<1>(_yAxisList, _yAxisList.iterator_for<BodyHooks::BeginY>(body), _overlapMap);
    insertionSort<1>(_yAxisList, _yAxisList.iterator_for<BodyHooks::EndY>(body), _overlapMap);
}

void Space::collide(Body& bodyA, Body& bodyB)
{
    // TODO
    (void)bodyA;
    (void)bodyB;
}
