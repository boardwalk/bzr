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
#ifndef BZR_PHYSICS_SPACE_H
#define BZR_PHYSICS_SPACE_H

#include "Body.h"
#include "ilist.h"
#include "Noncopyable.h"
#include <unordered_map>

template<class T>
inline void hash_combine(size_t& seed, const T& v)
{
    hash<T> hasher;
    seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

namespace std
{
    template<class T, class U>
    struct hash<pair<T, U>>
    {
        size_t operator()(const pair<T, U>& value)
        {
            auto result = size_t(0);
            hash_combine(result, value.first);
            hash_combine(result, value.second);
            return result;
        }
    };
}

class Space : Noncopyable
{
public:
    Space();
    void step(fp_t dt);
    void insert(Body& body);

private:
    typedef unordered_map<pair<Body*, Body*>, bool[2]> OverlapMap;

    bool step(fp_t dt, Body& body);
    void resort(Body& body);

    ilist<Body> _xAxisList;
    ilist<Body> _yAxisList;
    ilist<Body> _activeList;
    OverlapMap _overlapMap;
};

#endif