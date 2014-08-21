#include "Core.h"
#include "graphics/Renderer.h"
#include "Camera.h"
#include "Config.h"
#include "DatFile.h"
#include "LandblockManager.h"
#include "ResourceCache.h"
#include "util.h"

static const double STEP_RATE = 60.0;

static unique_ptr<Core> g_singleton;

void Core::go()
{
    assert(!g_singleton);
    g_singleton.reset(new Core());
    g_singleton->init();
    g_singleton->run();
    g_singleton->cleanup();
}

Core& Core::get()
{
    return *g_singleton;
}

Config& Core::config()
{
    return *_config;
}

const DatFile& Core::portalDat() const
{
    return *_portalDat;
}

const DatFile& Core::cellDat() const
{
    return *_cellDat;
}

const DatFile& Core::highresDat() const
{
    return *_highresDat;
}

ResourceCache& Core::resourceCache()
{
    return *_resourceCache;
}

LandblockManager& Core::landblockManager()
{
    return *_landblockManager;
}

Camera& Core::camera()
{
    return *_camera;
}

#ifndef HEADLESS
Renderer& Core::renderer()
{
    return *_renderer;
}
#endif

Core::Core() : _done(false), _config(nullptr)
{}

void Core::init()
{
    if(SDL_Init(SDL_INIT_TIMER) < 0)
    {
        throwSDLError();
    }

    _config.reset(new Config());
    _portalDat.reset(new DatFile("data/client_portal.dat"));
    _cellDat.reset(new DatFile("data/client_cell_1.dat"));
    _highresDat.reset(new DatFile("data/client_highres.dat"));
    _resourceCache.reset(new ResourceCache());
    _landblockManager.reset(new LandblockManager());
    _camera.reset(new Camera());
#ifndef HEADLESS
    _renderer.reset(new Renderer());
#endif
    _landblockManager->setCenter(LandblockId(0x31, 0xD6));
}

void Core::cleanup()
{
#ifndef HEADLESS
    _renderer.reset();
#endif
    _camera.reset();
    _landblockManager.reset();
    _resourceCache.reset();
    _portalDat.reset();
    _cellDat.reset();
    _highresDat.reset();
    _config.reset();

    SDL_Quit();
}

void Core::run()
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
        _renderer->render(interp);
#else
        // simulate ~83 without game logic
        SDL_Delay(12);
#endif
    }
}

void Core::handleEvents()
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
                // SDL does not respond normally to alt-f4 on Windows, so handle it ourselves
                if(event.key.keysym.sym == SDLK_F4 && (event.key.keysym.mod & KMOD_ALT) != 0)
                {
                    _done = true;
                }
#endif
                break;
        }
    }
}

void Core::step(double dt)
{
    const Uint8* state = SDL_GetKeyboardState(nullptr);

    auto lx = 0.0;
    auto ly = 0.0;

    if(state[SDL_SCANCODE_LEFT])
    {
        lx -= dt;
    }

    if(state[SDL_SCANCODE_RIGHT])
    {
        lx += dt;
    }

    if(state[SDL_SCANCODE_DOWN])
    {
        ly -= dt;
    }

    if(state[SDL_SCANCODE_UP])
    {
        ly += dt;
    }

    if(lx != 0.0 || ly != 0.0)
    {
        _camera->look(lx, ly);
    }

    auto mx = 0.0;
    auto my = 0.0;

    if(state[SDL_SCANCODE_A])
    {
        mx -= dt;
    }

    if(state[SDL_SCANCODE_D])
    {
        mx += dt;
    }

    if(state[SDL_SCANCODE_S])
    {
        my -= dt;
    }

    if(state[SDL_SCANCODE_W])
    {
        my += dt;
    }

    if(mx != 0 || my != 0)
    {
        _camera->move(mx, my);
    }

    auto& position = _camera->position();    
    auto id = _landblockManager->center();

    if(position.x < 0.0)
    {
        _camera->setPosition(Vec3(position.x + Landblock::LANDBLOCK_SIZE, position.y, position.z));
        _landblockManager->setCenter(LandblockId(id.x() - 1, id.y()));
    }

    if(position.x >= Landblock::LANDBLOCK_SIZE)
    {
        _camera->setPosition(Vec3(position.x - Landblock::LANDBLOCK_SIZE, position.y, position.z));
        _landblockManager->setCenter(LandblockId(id.x() + 1, id.y()));
    }

    if(position.y < 0.0)
    {
        _camera->setPosition(Vec3(position.x, position.y + Landblock::LANDBLOCK_SIZE, position.z));
        _landblockManager->setCenter(LandblockId(id.x(), id.y() - 1));
    }

    if(position.y >= Landblock::LANDBLOCK_SIZE)
    {
        _camera->setPosition(Vec3(position.x, position.y - Landblock::LANDBLOCK_SIZE, position.z));
        _landblockManager->setCenter(LandblockId(id.x(), id.y() + 1));
    }

    _camera->step(dt);
}
