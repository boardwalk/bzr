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
#include "PRNG.h"

double prng(uint32_t cell_x, uint32_t cell_y, uint32_t seed)
{
    uint32_t ival = 0x6C1AC587 * cell_y - 0x421BE3BD * cell_x -
        seed * (0x5111BFEF * cell_y * cell_x + 0x70892FB7);

    return static_cast<double>(ival) / static_cast<double>(UINT32_MAX);
}
