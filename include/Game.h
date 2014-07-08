#ifndef BZR_GAME_H
#define BZR_GAME_H

#include "gfx/Renderer.h"

class Game : Noncopyable
{
public:
    Game();
    ~Game();
    void run();

private:
    bool _running;
#ifndef HEADLESS
    unique_ptr<Renderer> _renderer;
#endif
};

#endif