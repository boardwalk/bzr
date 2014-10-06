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
#ifndef BZR_PRNG_H
#define BZR_PRNG_H

// if prng(x, y, RND_MID_DIAG) >= 0.5, the landcell's polygon is split NE/SW
static const int32_t RND_MID_DIAG       = 0x00000003;

// select scene floor(prng(x, y, RND_SCENE_PICK) * num_scenes) from scene type
static const int32_t RND_SCENE_PICK     = 0x00002bf9;

// if prng(x, y, RND_SCENE_FREQ + obj_num) < freq, show object
static const int32_t RND_SCENE_FREQ     = 0x00005b67;

// displaces an object prng(x, y RND_SCENE_DISP_X + obj_num) * displace_x units on the x axis
static const int32_t RND_SCENE_DISP_X   = 0x0000b2cd;

// displaces an object prng(x, y, RND_SCENE_DISP_Y + obj_num) * displace_y units on the y axis
static const int32_t RND_SCENE_DISP_Y   = 0x00011c0f;

// scales an object min_scale * pow(max_scale / min_scale, prng(x, y, RND_SCENE_SCALE_1 + obj_num))
static const int32_t RND_SCENE_SCALE1   = 0x00007f51;

// ??
static const int32_t RND_SCENE_SCALE2   = 0x000096a7;

// if prng(x, y, RND_SCENE_ROT) >= 0.75, (y, -x)
// if prng(...) >= 0.5, (-x, -y)
// if prng(...) >= 0.25, (-y, x)
// else, (y, x)  (sub_5A6D40)
static const int32_t RND_SCENE_ROT      = 0x0000e7eb;

// rotate a scene object prng(x, y, RND_SCENE_ROT + obj_num) * max_rot degrees
static const int32_t RND_SCENE_OBJROT   = 0x0000f697;

// @ 005005CB
static const int32_t RND_LAND_ROT       = 0x0000000d;

// @ 00500E51
static const int32_t RND_LAND_TEX       = 0x00000011;

static const int32_t RND_LAND_ALPHA2    = 0x00012071;
static const int32_t RND_LAND_ALPHA1    = 0x000002db;
static const int32_t RND_SKILL_APPRAISE = 0x00000013;
static const int32_t RND_TIME_WEATH1    = 0x0000a883;
static const int32_t RND_TIME_WEATH2    = 0x00013255;

// generates a number 0.0 to 1.0
double prng(uint32_t cell_x, uint32_t cell_y, uint32_t seed);

#endif