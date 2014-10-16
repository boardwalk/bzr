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
#include "net/ChecksumXorGenerator.h"

static const uint32_t kMaxCacheSize = 100;

ChecksumXorGenerator::ChecksumXorGenerator() : cacheBegin_(0)
{}

void ChecksumXorGenerator::init(uint32_t seed)
{
    memset(xorTable_, 0, sizeof(xorTable_));
    value0_ = seed;
    value1_ = seed;
    value2_ = seed;
    initTables();

    cache_.clear();
    cacheBegin_ = 2; // there is no sequence 0 or 1
}

uint32_t ChecksumXorGenerator::get(uint32_t sequence)
{
    if(cacheBegin_ == 0)
    {
        // not initialized
        return 0;
    }

    if(sequence < cacheBegin_)
    {
        // sequence already purged
        return 0;
    }

    if(sequence >= cacheBegin_ + kMaxCacheSize)
    {
        // sequence too far in future
        return 0;
    }

    while(sequence >= cacheBegin_ + cache_.size())
    {
        cache_.push_back(generate());
    }

    return cache_[sequence - cacheBegin_];
}

void ChecksumXorGenerator::purge(uint32_t sequence)
{
    if(cacheBegin_ == 0)
    {
        // not initialized
        return;
    }

    if(sequence >= cacheBegin_ + cache_.size())
    {
        // get not successful for sequence yet
        return;
    }

    while(sequence >= cacheBegin_)
    {
        cache_.pop_front();
        cacheBegin_++;
    }
}

uint32_t ChecksumXorGenerator::generate()
{
    uint32_t value = xorTable_[counter_];

    if(counter_ > 0)
    {
        counter_--;
    }
    else
    {
        scramble();
        counter_ = 255;
    }

    return value;
}

void ChecksumXorGenerator::initTables()
{
    uint32_t xorvals[8];

    for(int i = 0; i < 8; i++)
    {
        xorvals[i] = 0x9E3779B9;
    }

    for(int i = 0; i < 4; i++)
    {
        initMix(xorvals);
    }

    for(int i = 0; i < 256; i += 8)
    {
        for(int j = 0; j < 8; j++)
        {
            xorvals[j] += xorTable_[i + j];
        }

        initMix(xorvals);

        for(int j = 0; j < 8; j++)
        {
            unkTable_[i + j] = xorvals[j];
        }
    }

    for(int i = 0; i < 256; i += 8)
    {
        for(int j = 0; j < 8; j++)
        {
            xorvals[j] += unkTable_[i + j];
        }

        initMix(xorvals);

        for(int j = 0; j < 8; j++)
        {
            unkTable_[i + j] = xorvals[j];
        }
    }

    scramble();
    counter_ = 255;
}

void ChecksumXorGenerator::initMix(uint32_t* xorvals)
{
#define ROUND(base, shift) \
    xorvals[base] ^= xorvals[(base + 1) & 7] shift; \
    xorvals[(base + 3) & 7] += xorvals[base]; \
    xorvals[(base + 1) & 7] += xorvals[(base + 2) & 7];

    ROUND(0, << 0x0B);
    ROUND(1, >> 0x02);
    ROUND(2, << 0x08);
    ROUND(3, >> 0x10);
    ROUND(4, << 0x0A);
    ROUND(5, >> 0x04);
    ROUND(6, << 0x08);
    ROUND(7, >> 0x09);

#undef ROUND
}

void ChecksumXorGenerator::scramble()
{
    uint32_t* local_unk = unkTable_;
    uint32_t* local_xor = xorTable_;
    uint32_t key0 = value0_;
    value2_++;
    uint32_t key2 = value1_ + value2_;
    uint32_t* lc_unk0 = local_unk;
    uint32_t* lc_unk200 = lc_unk0 + 128;
    uint32_t* lc_unk0_stop_point = lc_unk200;
    uint32_t var_18;
    uint32_t var_1c;

    while(lc_unk0 < lc_unk0_stop_point)
    {
        scrambleRound(key0 << 0x0D, &key0, &key2, &local_unk, &lc_unk0, &lc_unk200, &local_xor, &var_18, &var_1c);
        scrambleRound(key0 >> 0x06, &key0, &key2, &local_unk, &lc_unk0, &lc_unk200, &local_xor, &var_18, &var_1c);
        scrambleRound(key0 << 0x02, &key0, &key2, &local_unk, &lc_unk0, &lc_unk200, &local_xor, &var_18, &var_1c);
        scrambleRound(key0 >> 0x10, &key0, &key2, &local_unk, &lc_unk0, &lc_unk200, &local_xor, &var_18, &var_1c);
    }

    lc_unk200 = local_unk;

    while(lc_unk200 < lc_unk0_stop_point)
    {
        scrambleRound(key0 << 0x0D, &key0, &key2, &local_unk, &lc_unk0, &lc_unk200, &local_xor, &var_18, &var_1c);
        scrambleRound(key0 >> 0x06, &key0, &key2, &local_unk, &lc_unk0, &lc_unk200, &local_xor, &var_18, &var_1c);
        scrambleRound(key0 << 0x02, &key0, &key2, &local_unk, &lc_unk0, &lc_unk200, &local_xor, &var_18, &var_1c);
        scrambleRound(key0 >> 0x10, &key0, &key2, &local_unk, &lc_unk0, &lc_unk200, &local_xor, &var_18, &var_1c);
    }

    value1_ = key2;
    value0_ = key0;
}

void ChecksumXorGenerator::scrambleRound(
        uint32_t shiftedVal,
        uint32_t* key0_ptr,
        uint32_t* key2_ptr,
        uint32_t** localunk_ptr,
        uint32_t** lc_unk0_ptr,
        uint32_t** lc_unk200_ptr,
        uint32_t** localxor_ptr,
        uint32_t* var_18_ptr,
        uint32_t* var_1c_ptr)
{
    *var_18_ptr = **lc_unk0_ptr;
    *key0_ptr = (*key0_ptr ^ shiftedVal) + **lc_unk200_ptr;
    *lc_unk200_ptr = *lc_unk200_ptr + 1;
    uint32_t res = Crazy_XOR_01(*localunk_ptr, *var_18_ptr);
    *var_1c_ptr = res + *key0_ptr + *key2_ptr;
    **lc_unk0_ptr = *var_1c_ptr;
    *lc_unk0_ptr = *lc_unk0_ptr + 1;
    res = Crazy_XOR_01(*localunk_ptr, *var_1c_ptr >> 8);
    *key2_ptr = res + *var_18_ptr;
    **localxor_ptr = *key2_ptr;
    *localxor_ptr = *localxor_ptr + 1;
}

uint32_t ChecksumXorGenerator::Crazy_XOR_01(const uint32_t* data, uint32_t index)
{
    return *(const uint32_t*)((const uint8_t*)data + (index & 0x3FC));
}
