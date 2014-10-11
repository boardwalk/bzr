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
#include "resource/Region.h"
#include "Camera.h"
#include "Config.h"
#include "DatFile.h"
#include "Land.h"
#include "LandcellManager.h"
#include "ObjectManager.h"
#include "ResourceCache.h"
#include "util.h"

static const fp_t kStepRate = 60.0;

static unique_ptr<Core> g_singleton;

void Core::go()
{
    assert(!g_singleton);
    g_singleton.reset(new Core{});
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
    return *config_;
}

DatFile& Core::portalDat()
{
    return *portalDat_;
}

DatFile& Core::cellDat()
{
    return *cellDat_;
}

DatFile& Core::highresDat()
{
    return *highresDat_;
}

ResourceCache& Core::resourceCache()
{
    return *resourceCache_;
}

LandcellManager& Core::landcellManager()
{
    return *landcellManager_;
}

ObjectManager& Core::objectManager()
{
    return *objectManager_;
}

const Region& Core::region() const
{
    return region_->cast<Region>();
}

Camera& Core::camera()
{
    return *camera_;
}

#ifndef HEADLESS
Renderer& Core::renderer()
{
    return *renderer_;
}
#endif

ObjectId Core::playerId() const
{
    return playerId_;
}

void Core::setPlayerId(ObjectId playerId)
{
    playerId_ = playerId;
}

Core::Core() : done_(false)
// TEMPORARY
#ifndef HEADLESS
, modelId_(0x02000120)
#endif
{}

void Core::init()
{
    if(SDL_Init(SDL_INIT_TIMER) < 0)
    {
        throwSDLError();
    }

    config_.reset(new Config{});
    portalDat_.reset(new DatFile{"data/client_portal.dat"});
    cellDat_.reset(new DatFile{"data/client_cell_1.dat"});
    highresDat_.reset(new DatFile{"data/client_highres.dat"});
    resourceCache_.reset(new ResourceCache{});
    landcellManager_.reset(new LandcellManager{});
    objectManager_.reset(new ObjectManager{});
    region_ = resourceCache_->get(0x13000000);
    camera_.reset(new Camera{});
#ifndef HEADLESS
    renderer_.reset(new Renderer{});
    renderer_->init();
#endif
    landcellManager_->setCenter(LandcellId(0x31, 0xD6));

#if 0
    for(uint32_t resourceId : portalDat_->list())
    {
        if((resourceId & 0xFF000000) > 0x0D000000)
        {
            continue;
        }

        try
        {
            resourceCache_->get(resourceId);

            printf("%08x OK\n", resourceId);
        }
        catch(runtime_error& e)
        {
            printf("%08x FAIL %s\n", resourceId, e.what());
        }
    }
#endif

    setPlayerId(ObjectId{1});
}

void Core::cleanup()
{
#ifndef HEADLESS
    renderer_.reset();
#endif
    camera_.reset();
    objectManager_.reset();
    landcellManager_.reset();
    resourceCache_.reset();
    portalDat_.reset();
    cellDat_.reset();
    highresDat_.reset();
    config_.reset();

    SDL_Quit();
}

void Core::run()
{
    uint64_t frequency = SDL_GetPerformanceFrequency();
    uint64_t fixedStep = frequency / static_cast<uint64_t>(kStepRate);
    uint64_t maxTotalDelta = fixedStep * 6;
    uint64_t stepTime = SDL_GetPerformanceCounter();

    while(!done_)
    {
        uint64_t loopTime = SDL_GetPerformanceCounter();

        if(loopTime > stepTime + maxTotalDelta)
        {
            stepTime = loopTime - maxTotalDelta;
        }

        while(loopTime >= stepTime + fixedStep)
        {
            handleEvents();
            step(fp_t(1.0) / kStepRate);
            stepTime += fixedStep;
        }

#ifndef HEADLESS
        fp_t interp = static_cast<fp_t>(loopTime - stepTime) / static_cast<fp_t>(frequency);
        renderer_->render(interp);
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
                done_ = true;
                break;

            case SDL_KEYDOWN:
#ifdef WIN32
                // SDL does not respond normally to alt-f4 on Windows, so handle it ourselves
                if(event.key.keysym.sym == SDLK_F4 && (event.key.keysym.mod & KMOD_ALT) != 0)
                {
                    done_ = true;
                }
#endif
                // TEMPORARY
#ifndef HEADLESS
                if(event.key.keysym.sym == SDLK_z)
                {
                    while(modelId_ > 0x02000000)
                    {
                        modelId_--;

                        if(!portalDat_->read(modelId_).empty())
                        {
                            newModel = true;
                            break;
                        }
                    }
                }

                if(event.key.keysym.sym == SDLK_x)
                {
                    while(modelId_ < 0x02005000)
                    {
                        modelId_++;

                        if(!portalDat_->read(modelId_).empty())
                        {
                            newModel = true;
                            break;
                        }
                    }
                }
#endif

                break;
        }
    }

    // TEMPORARY
#ifndef HEADLESS
    if(newModel)
    {
        printf("Loading model %08x\n", modelId_);

        Object& object = (*objectManager_)[ObjectId{1}];

        object.setModel(resourceCache_->get(modelId_));

        object.setLandcellId(landcellManager_->center());

        Location location;
        location.position = glm::vec3{92.0, 92.0, 0.0};
        location.rotation = glm::quat{1.0, 0.0, 0.0, 0.0};
        object.setLocation(location);
    }
#endif
}

void Core::step(fp_t dt)
{
    const Uint8* state = SDL_GetKeyboardState(nullptr);

    fp_t speed = 0.0;

    if(state[SDL_SCANCODE_LSHIFT])
    {
        speed = 100.0;
    }
    else
    {
        speed = 10.0;
    }

    camera_->setSpeed(speed);

    fp_t lx = 0.0;
    fp_t ly = 0.0;

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
        camera_->look(lx, ly);
    }

    fp_t mx = 0.0;
    fp_t my = 0.0;

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
        camera_->move(mx, my);
    }

    const glm::vec3& position = camera_->position();
    LandcellId id = landcellManager_->center();

    if(position.x < 0.0)
    {
        camera_->setPosition(glm::vec3(position.x + Land::kBlockSize, position.y, position.z));
        landcellManager_->setCenter(LandcellId(id.x() - 1, id.y()));
    }

    if(position.x >= Land::kBlockSize)
    {
        camera_->setPosition(glm::vec3(position.x - Land::kBlockSize, position.y, position.z));
        landcellManager_->setCenter(LandcellId(id.x() + 1, id.y()));
    }

    if(position.y < 0.0)
    {
        camera_->setPosition(glm::vec3(position.x, position.y + Land::kBlockSize, position.z));
        landcellManager_->setCenter(LandcellId(id.x(), id.y() - 1));
    }

    if(position.y >= Land::kBlockSize)
    {
        camera_->setPosition(glm::vec3(position.x, position.y - Land::kBlockSize, position.z));
        landcellManager_->setCenter(LandcellId(id.x(), id.y() + 1));
    }

    camera_->step(dt);
}
