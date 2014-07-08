#include "Game.h"
#include <cstdio>
#include <cstdlib>
#include <memory>

int main()
{
    try
    {
        // may be too large for the stack
        make_unique<Game>()->run();
    }
    catch(const runtime_error& e)
    {
        fprintf(stderr, "An error ocurred: %s\n", e.what());
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
