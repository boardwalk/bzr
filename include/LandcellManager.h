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
#ifndef BZR_LANDCELLMANAGER_H
#define BZR_LANDCELLMANAGER_H

#include "Landcell.h"
#include "Noncopyable.h"
#include <unordered_map>

class LandcellManager : Noncopyable
{
public:
    typedef unordered_map<LandcellId, unique_ptr<Landcell>> container_type;
    typedef container_type::iterator iterator;

    LandcellManager();

    void setCenter(LandcellId center);
    LandcellId center() const;

    iterator find(LandcellId id);
    iterator begin();
    iterator end();

private:
    void load();

    container_type data_;
    LandcellId center_;
    int radius_;
};

#endif
