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
#ifndef BZR_CORE_H
#define BZR_CORE_H

#include "ObjectId.h"

class Camera;
class Config;
class DatFile;
class LandcellManager;
class ObjectManager;
class Renderer;
class ResourceCache;

class Core
{
public:
    static void go();
    static Core& get();

    Config& config();
    DatFile& portalDat();
    DatFile& cellDat();
    DatFile& highresDat();
    ResourceCache& resourceCache();
    LandcellManager& landcellManager();
    ObjectManager& objectManager();
    Camera& camera();
#ifndef HEADLESS
    Renderer& renderer();
#endif
    ObjectId playerId() const;
    void setPlayerId(ObjectId playerId);

private:
    Core();

    void init();
    void cleanup();

    void run();
    void handleEvents();
    void step(fp_t dt);

    bool done_;
    unique_ptr<Config> config_;
    unique_ptr<DatFile> portalDat_;
    unique_ptr<DatFile> cellDat_;
    unique_ptr<DatFile> highresDat_;
    unique_ptr<ResourceCache> resourceCache_;
    unique_ptr<LandcellManager> landcellManager_;
    unique_ptr<ObjectManager> objectManager_;
    unique_ptr<Camera> camera_;
#ifndef HEADLESS
    unique_ptr<Renderer> renderer_;
#endif
    ObjectId playerId_;
    // TEMPORARY
    uint32_t modelId_;
};

#endif
