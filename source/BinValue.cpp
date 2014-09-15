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
#include "BinValue.h"

const BinSchema& BinValue::schema() const
{
    return _schema;
}

const BinValue& operator[](const string& key) const
{
    assert(_schema.type() == BinSchema::Structure);
    // TODO
    return *this;
}

const BinValue& operator[](size_t key) const
{
    assert(_schema.type() == BinSchema::Array);

    if(key >= _length)
    {
        throw out_of_range("index out of range for array");
    }

    // TODO
    return *this;
}

