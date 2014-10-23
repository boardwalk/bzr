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
#include "Core.h"
#include <SDL_main.h>
#include <signal.h>
#include <cstdio>
#include <cstdlib>

static void sighandler(int)
{
    Core::get().stop();
}

int main(int argc, char* argv[])
{
    // SDL_main requires we have these arguments
    UNUSED(argc);
    UNUSED(argv);

    signal(SIGINT, sighandler);

    try
    {
        Core::execute();
    }
    catch(const runtime_error& e)
    {
        fprintf(stderr, "An error ocurred: %s\n", e.what());
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
