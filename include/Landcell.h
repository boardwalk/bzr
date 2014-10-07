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
#ifndef BZR_LANDCELL_H
#define BZR_LANDCELL_H

#include "Destructable.h"
#include "StaticObject.h"
#include "LandcellId.h"
#include "Noncopyable.h"

class Landcell : Noncopyable
{
public:
    virtual ~Landcell();

    virtual LandcellId id() const = 0;
    const vector<StaticObject>& staticObjects() const;
    unique_ptr<Destructable>& renderData() const;

protected:
    vector<StaticObject> staticObjects_;

private:
    mutable unique_ptr<Destructable> renderData_;
};

#endif
