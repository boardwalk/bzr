#include "Core.h"
#include <SDL_main.h>
#include <cstdio>
#include <cstdlib>

int main(int argc, char* argv[])
{
    // SDL_main requires we have these arguments
    (void)argc;
    (void)argv;

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
