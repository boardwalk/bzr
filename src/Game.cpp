#include "Game.h"
#include "Landblock.h"
#include "gfx/Renderer.h"
#include "DatFile.h"
#include "util.h"
#include <SDL.h>

static const double STEP_RATE = 60.0;

Game::Game() : _done(false)
{
    _portalDat.reset(new DatFile("data/client_portal.dat"));
    _cellDat.reset(new DatFile("data/client_cell_1.dat"));

    if(SDL_Init(SDL_INIT_TIMER) < 0)
    {
        throwSDLError();
    }

#ifndef HEADLESS
    _renderer.reset(new Renderer());
#endif

    auto landblockData = _cellDat->read(0x2343FFFF);
    _landblock.reset(new Landblock(landblockData.data(), landblockData.size()));
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
    uint64_t frequency = SDL_GetPerformanceFrequency();
    uint64_t fixedStep = frequency / uint64_t(STEP_RATE);
    uint64_t maxTotalDelta = fixedStep * 6;
    uint64_t time = SDL_GetPerformanceCounter();

    while(!_done)
    {
        uint64_t loopTime = SDL_GetPerformanceCounter();

        if(loopTime > time + maxTotalDelta)
        {
            time = loopTime - maxTotalDelta;
        }

        while(loopTime >= time + fixedStep)
        {
            handleEvents();
            step(1.0 / STEP_RATE);
            time += fixedStep;
        }

#ifndef HEADLESS
        double interp = (double)(loopTime - time) / (double)frequency;
        _renderer->render(*this, interp);
#else
        // simulate ~83 without game logic
        SDL_Delay(12);
#endif
    }
}

const DatFile& Game::portalDat() const
{
    return *_portalDat;
}

const DatFile& Game::cellDat() const
{
    return *_cellDat;
}

Landblock& Game::landblock()
{
    return *_landblock;
}

void Game::handleEvents()
{
    SDL_Event event;

    while(SDL_PollEvent(&event) != 0)
    {
        switch(event.type)
        {
            case SDL_QUIT:
                _done = true;
                break;

            case SDL_KEYDOWN:
#ifdef WIN32
                // SDL does not response normally to alt-f4 on Windows, so handle it ourselves
                if(event.key.keysym.sym == SDLK_F4 && (event.key.keysym.mod & KMOD_ALT) != 0)
                {
                    _done = true;
                }
#endif
                break;
        }
    }
}

void Game::step(double dt)
{
    // TODO
}

