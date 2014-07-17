#include "Core.h"
#include <SDL_main.h>
#include <cstdio>
#include <cstdlib>
#include <stdexcept>

int main(int argc, char* argv[])
{
    try
    {
        Core::go();
    }
    catch(const runtime_error& e)
    {
        fprintf(stderr, "An error ocurred: %s\n", e.what());
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
