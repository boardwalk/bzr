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
#ifndef BZR_STRUCTURE_H
#define BZR_STRUCTURE_H

#include "Landcell.h"
#include "Location.h"

struct Environment;

class Structure : public Landcell
{
public:
    Structure(const void* data, size_t size);

    LandcellId id() const override;
    const Location& location() const;
    const vector<ResourcePtr>& surfaces() const;
    const Environment& environment() const;
    uint16_t partNum() const;

private:
    LandcellId id_;
    Location location_;
    vector<ResourcePtr> surfaces_;
    ResourcePtr environment_;
    uint16_t partNum_;
};

#endif