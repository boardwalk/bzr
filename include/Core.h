#ifndef BZR_CORE_H
#define BZR_CORE_H

class Camera;
class Config;
class DatFile;
class LandblockManager;
class Renderer;
class ResourceCache;

class Core
{
public:
    static void go();
    static Core& get();

    Config& config();
    const DatFile& portalDat() const;
    const DatFile& cellDat() const;
    const DatFile& highresDat() const;
    ResourceCache& resourceCache();
    LandblockManager& landblockManager();
    Camera& camera();

private:
    Core();

    void init();
    void cleanup();

    void run();
    void handleEvents();
    void step(double dt);

    bool _done;
    unique_ptr<Config> _config;
    unique_ptr<DatFile> _portalDat;
    unique_ptr<DatFile> _cellDat;
    unique_ptr<DatFile> _highresDat;
    unique_ptr<ResourceCache> _resourceCache;
    unique_ptr<LandblockManager> _landblockManager;
    unique_ptr<Camera> _camera;
#ifndef HEADLESS
    unique_ptr<Renderer> _renderer;
#endif
};

#endif
