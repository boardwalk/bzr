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
#include <algorithm>

static const auto GRAVITY_ACCEL = fp_t(-9.81);
static const auto TERMINAL_VELOCITY = fp_t(-54.0);
static const auto REST_EPSILON = fp_t(0.002);

static fp_t getBoundValueX(ilist<Body>::iterator it)
{
    if(it.on_hook<BodyHooks::BeginX>())
    {
        return it->bounds().min.x;
    }
    
    if(it.on_hook<BodyHooks::EndX>())
    {
        return it->bounds().max.x;
    }
    
    assert(!"Iterator for X axis associated with unknown hook");
    return 0.0;
}

static fp_t getBoundValueY(ilist<Body>::iterator it)
{
    if(it.on_hook<BodyHooks::BeginY>())
    {
        return it->bounds().min.y;
    }

    if(it.on_hook<BodyHooks::EndY>())
    {
        return it->bounds().max.y;
    }

    assert(!"Iterator for Y axis associated with unknown hook");
    return 0.0;
}

template<int Axis>
bool compareBody1D(ilist<Body>::iterator, ilist<Body>::iterator);

template<>
bool compareBody1D<0>(ilist<Body>::iterator a, ilist<Body>::iterator b)
{
    fp_t aValue = a->location().offset.x + getBoundValueX(a);
    fp_t bValue = b->location().offset.x + getBoundValueX(b);
    int blockDiff = static_cast<int>(b->location().landcell.x()) - static_cast<int>(a->location().landcell.x());
    return aValue < bValue + blockDiff * fp_t(192.0);
}

template<>
bool compareBody1D<1>(ilist<Body>::iterator a, ilist<Body>::iterator b)
{
    fp_t aValue = a->location().offset.y + getBoundValueY(a);
    fp_t bValue = b->location().offset.y + getBoundValueY(b);
    int blockDiff = static_cast<int>(b->location().landcell.y()) - static_cast<int>(a->location().landcell.y());
    return aValue < bValue + blockDiff * fp_t(192.0);
}

template<int Axis>
static void insertionSort(ilist<Body>& container, ilist<Body>::iterator bodyIt)
{
    // a < b, compare(a, b)
    // a >= b, !compare(a, b)
    // a > b, compare(b, a)
    // a <= b, !compare(b, a)
    // FIXME make sure this is correct

    auto nextIt = container.erase(bodyIt);

    while(nextIt != container.end() && !compareBody1D<Axis>(nextIt, bodyIt)) // bodyIt <= nextIt
    {
        pair<Body*, Body*> key = { &*nextIt, &*bodyIt };

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

        if(bodyIt.on_hook<BodyHooks::BeginX>() && nextIt.on_hook<BodyHooks::EndX>())
        {
            //_overlapMap[key][axis] = false;
        }
        else if(bodyIt.on_hook<BodyHooks::EndX>() && nextIt.on_hook<BodyHooks::BeginX>())
        {
            //_overlapMap[key] = true;
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

    auto oldLocation = body.location();

    auto newLocation = body.location();
    newLocation.offset += body.velocity() * dt;

    body.setLocation(newLocation);
    resort(body);

    //auto beginXIt = decltype(_beginXList)::iterator(&body);

    //while(beginXIt != _beginXList.end())
    //{
        // exit condition
    //    ++beginXIt;
    //}

    //auto segment = LineSegment();
    //segment.begin = body.location().normalize();
    //segment.end = segment.begin + body.velocity() * dt;

    return true;
}

void Space::resort(Body& body)
{
    insertionSort<0>(_xAxisList, _xAxisList.iterator_for<BodyHooks::BeginX>(body));
    insertionSort<0>(_xAxisList, _xAxisList.iterator_for<BodyHooks::EndX>(body));
    insertionSort<1>(_yAxisList, _yAxisList.iterator_for<BodyHooks::BeginY>(body));
    insertionSort<1>(_yAxisList, _yAxisList.iterator_for<BodyHooks::EndY>(body));
}
