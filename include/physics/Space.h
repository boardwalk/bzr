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

class Space : Noncopyable
{
public:
    void step(fp_t dt);

    void insert(Body& body);

private:
    bool step(fp_t dt, Body& body);

    ilist<Body, BeginXTag> _beginXList;
    ilist<Body, BeginYTag> _beginYList;
    ilist<Body, EndXTag> _endXList;
    ilist<Body, EndYTag> _endYList;
    ilist<Body, ActiveTag> _activeList;
};

#endif