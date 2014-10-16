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
#ifndef BZR_CHECKSUMXORGENERATOR_H
#define BZR_CHECKSUMXORGENERATOR_H

#include <deque>

class ChecksumXorGenerator
{
public:
    ChecksumXorGenerator();

    void init(uint32_t seed);
    uint32_t get(uint32_t sequence);
    void purge(uint32_t sequence);

private:
    uint32_t generate();
    void initTables();
    static void initMix(uint32_t* xorvals);
    void scramble();
    static void scrambleRound(uint32_t shiftedVal, uint32_t* key0_ptr, uint32_t* key2_ptr, uint32_t** localunk_ptr, uint32_t** lc_unk0_ptr, uint32_t** lc_unk200_ptr, uint32_t** localxor_ptr, uint32_t* var_18_ptr, uint32_t* var_1c_ptr);
    static uint32_t Crazy_XOR_01(const uint32_t* data, uint32_t index);

    uint32_t counter_;
    uint32_t xorTable_[256];
    uint32_t unkTable_[256];
    uint32_t value0_;
    uint32_t value1_;
    uint32_t value2_;

    uint32_t cacheBegin_;
    deque<uint32_t> cache_;
};

#endif