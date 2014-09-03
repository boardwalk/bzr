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
#ifndef HEADLESS
#include "graphics/Renderer.h"
#endif
#include "physics/Space.h"
#include "Camera.h"
#include "Config.h"
#include "DatFile.h"
#include "LandblockManager.h"
#include "ObjectManager.h"
#include "ResourceCache.h"
#include "util.h"

static const auto STEP_RATE = fp_t(60.0);

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

DatFile& Core::portalDat()
{
    return *_portalDat;
}

DatFile& Core::cellDat()
{
    return *_cellDat;
}

DatFile& Core::highresDat()
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

ObjectManager& Core::objectManager()
{
    return *_objectManager;
}

Space& Core::space()
{
    return *_space;
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

ObjectId Core::playerId() const
{
    return _playerId;
}

void Core::setPlayerId(ObjectId playerId)
{
    _playerId = playerId;
}

Core::Core() : _done(false) /* TEMPORARY */, _modelId(0x02000120), _submodelNum(0)
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
    _objectManager.reset(new ObjectManager());
    _space.reset(new Space());
    _camera.reset(new Camera());
#ifndef HEADLESS
    _renderer.reset(new Renderer());
    _renderer->init();
#endif
    _landblockManager->setCenter(LandblockId(0x31, 0xD6));
    
    (*_objectManager)[ObjectId(1)];
    setPlayerId(ObjectId(1));
}

void Core::cleanup()
{
#ifndef HEADLESS
    _renderer.reset();
#endif
    _camera.reset();
    _space.reset();
    _objectManager.reset();
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
            step(fp_t(1.0) / STEP_RATE);
            time += fixedStep;
        }

#ifndef HEADLESS
        fp_t interp = fp_t(loopTime - time) / fp_t(frequency);
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

#ifndef HEADLESS
    bool newModel = false; // TEMPORARY
#endif

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
                // TEMPORARY
#ifndef HEADLESS
                if(event.key.keysym.sym == SDLK_z)
                {
                    while(_modelId > 0x02000000)
                    {
                        _modelId--;

                        if(!_portalDat->read(_modelId).empty())
                        {
                            newModel = true;
                            break;
                        }
                    }
                }

                if(event.key.keysym.sym == SDLK_x)
                {
                    while(_modelId < 0x02005000)
                    {
                        _modelId++;

                        if(!_portalDat->read(_modelId).empty())
                        {
                            newModel = true;
                            break;
                        }
                    }
                }

                if(event.key.keysym.sym == SDLK_c)
                {
                    _submodelNum--;
                    printf("Submodel num %d\n", _submodelNum);
                    _renderer->setSubmodelNum(_submodelNum);
                }

                if(event.key.keysym.sym == SDLK_v)
                {
                    _submodelNum++;
                    printf("Submodel num %d\n", _submodelNum);
                    _renderer->setSubmodelNum(_submodelNum);
                }
#endif

                break;
        }
    }

    // TEMPORARY
#ifndef HEADLESS
    if(newModel)
    {
        printf("Loading model %08x\n", _modelId);
        _renderer->setModel( _resourceCache->get(_modelId) );
    }
#endif
}

void Core::step(fp_t dt)
{
    const Uint8* state = SDL_GetKeyboardState(nullptr);

    auto speed = fp_t(0.0);

    if(state[SDL_SCANCODE_LSHIFT])
    {
        speed = 100.0;
    }
    else
    {
        speed = 10.0;
    }

    _camera->setSpeed(speed);

    auto lx = fp_t(0.0);
    auto ly = fp_t(0.0);

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

    auto mx = fp_t(0.0);
    auto my = fp_t(0.0);

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
        _camera->setPosition(glm::vec3(position.x + Landblock::LANDBLOCK_SIZE, position.y, position.z));
        _landblockManager->setCenter(LandblockId(id.x() - 1, id.y()));
    }

    if(position.x >= Landblock::LANDBLOCK_SIZE)
    {
        _camera->setPosition(glm::vec3(position.x - Landblock::LANDBLOCK_SIZE, position.y, position.z));
        _landblockManager->setCenter(LandblockId(id.x() + 1, id.y()));
    }

    if(position.y < 0.0)
    {
        _camera->setPosition(glm::vec3(position.x, position.y + Landblock::LANDBLOCK_SIZE, position.z));
        _landblockManager->setCenter(LandblockId(id.x(), id.y() - 1));
    }

    if(position.y >= Landblock::LANDBLOCK_SIZE)
    {
        _camera->setPosition(glm::vec3(position.x, position.y - Landblock::LANDBLOCK_SIZE, position.z));
        _landblockManager->setCenter(LandblockId(id.x(), id.y() + 1));
    }

    _space->step(dt);
    _camera->step(dt);
}
