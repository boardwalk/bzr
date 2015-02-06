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
#include "eui.h"

/*
 * Constraint language
 * w, h: natural width and height of the current control in pts
 * pw, ph: natural with and height of the parent control in pts
 * 
 */

#ifdef _MSC_VER
#define NOINLINE __declspec(noinline)
#define RETURNADDR() _ReturnAddress()
#else
#define NOINLINE __attribute__((noinline))
#define RETURNADDR() __builtin_return_address(0)
#endif

namespace {

eui::Context* g_ctx = nullptr;

} // anonymous namespace

namespace eui {

Element::Element()
{
    name.first = reinterpret_cast<intptr_t>(this);
    name.second = g_ctx->addrCount[id.first]++;
}

void setContext(Context* ctx)
{
    g_ctx = ctx;
}

NOINLINE intptr_t pc()
{
    return reinterpret_cast<intptr_t>(RETURNADDR());
}

void pushId(intptr_t id)
{
    g_ctx->idStack.push(id);
}

void popId()
{
    g_ctx->idStack.pop();
}

} // namespace eui