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
#ifndef BZR_IOUTIL_H
#define BZR_IOUTIL_H

#include <iomanip>
#include <ostream>
#include <type_traits>

template<class T>
struct HexWrapper { typename enable_if<is_unsigned<T>::value, T>::type value; };

template<class T>
inline ostream& operator<<(ostream& os, HexWrapper<T> wrapped)
{
    for(size_t i = 1; i <= sizeof(T) * 2; i++)
    {
        int digit = (wrapped.value >> (sizeof(T) * 8 - i * 4)) & 0xF;
        os << "0123456789ABCDEF"[digit];
    }

    return os << "h";
}

template<class T>
inline HexWrapper<T> hexn(T value)
{
    return HexWrapper<T> { value };
}

#endif
