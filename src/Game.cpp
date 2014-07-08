#include "Game.h"
#include "util.h"
#include <SDL.h>

Game::Game() : _running(false)
{
    if(SDL_Init(SDL_INIT_TIMER) < 0)
    {
        throwSDLError();
    }

#ifndef HEADLESS
    _renderer = make_unique<Renderer>();
#endif
}

Game::~Game()
{
#ifndef HEADLESS
    _renderer.reset();
#endif

    SDL_Quit();
}

void Game::run()
{
    while(_running)
    {
        SDL_Event event;

        while(SDL_PollEvent(&event) != 0)
        {
            switch(event.type)
            {
                case SDL_QUIT:
                    _running = false;
                    break;
            }

#ifndef HEADLESS
            _renderer->render();
#endif
        }
    }
}

