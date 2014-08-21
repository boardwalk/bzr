#include "util.h"

void throwSDLError()
{
    throw runtime_error(string("SDL_Init failed: ") + SDL_GetError());
}
