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
#include "ObjectManager.h"

void ObjectManager::setPlayerId(ObjectId playerId)
{
    playerId_ = playerId;
}

Object& ObjectManager::player()
{
    if(playerId_.value() == 0)
    {
        throw runtime_error("player id not set");
    }

    return (*this)[playerId_];
}

Object& ObjectManager::operator[](ObjectId id)
{
    unique_ptr<Object>& ptr = data_[id];

    if(!ptr)
    {
        ptr.reset(new Object());
    }

    return *ptr;
}

ObjectManager::iterator ObjectManager::find(ObjectId id)
{
    return data_.find(id);
}

ObjectManager::iterator ObjectManager::begin()
{
    return data_.begin();
}

ObjectManager::iterator ObjectManager::end()
{
    return data_.end();
}
