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
class LandblockManager;
class ObjectManager;
class Renderer;
class ResourceCache;
class Space;

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
    LandblockManager& landblockManager();
    ObjectManager& objectManager();
    Space& space();
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

    bool _done;
    unique_ptr<Config> _config;
    unique_ptr<DatFile> _portalDat;
    unique_ptr<DatFile> _cellDat;
    unique_ptr<DatFile> _highresDat;
    unique_ptr<ResourceCache> _resourceCache;
    unique_ptr<LandblockManager> _landblockManager;
    unique_ptr<ObjectManager> _objectManager;
    unique_ptr<Space> _space;
    unique_ptr<Camera> _camera;
#ifndef HEADLESS
    unique_ptr<Renderer> _renderer;
#endif
    ObjectId _playerId;
    // TEMPORARY
    uint32_t _modelId;
    uint32_t _submodelNum;
};

#endif
