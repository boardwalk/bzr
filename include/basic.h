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
#ifndef BZR_BASIC_H
#define BZR_BASIC_H

#include <SDL.h>

#ifndef HEADLESS
#ifdef _MSC_VER
#include <GL/glew.h>
#else
#define GL_GLEXT_PROTOTYPES
#include <SDL_opengl.h>
#endif
#endif

#define GLM_FORCE_RADIANS
#include <glm/gtc/quaternion.hpp> // includes mat3, mat4, vec3, vec4

#include <cassert>
#include <cstdint>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#ifdef __GNUC__
#define PACK(decl) decl __attribute__((__packed__))
#elif _MSC_VER
#define PACK(decl) __pragma(pack(push, 1)) decl __pragma(pack(pop))
#else
#error Implement PACK for this compiler.
#endif

#define UNUSED(x) ((void)x)

using namespace std;

typedef glm::float_t fp_t;

inline fp_t pi() { return glm::pi<fp_t>(); }

#endif
