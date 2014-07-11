#ifndef BZR_CORE_H
#define BZR_CORE_H

#include "Noncopyable.h"

class DatFile;
class Camera;
class Landblock;
class Renderer;

class Core : Noncopyable
{
public:
    ~Core();

    static void init();
    static void cleanup();
    static Core& get();

    void run();

    const DatFile& portalDat() const;
    const DatFile& cellDat() const;
    const DatFile& highresDat() const;

    const Camera& camera() const;

    Landblock& landblock();

private:
    Core();

    void handleEvents();
    void step(double dt);

    bool _done;
    unique_ptr<DatFile> _portalDat;
    unique_ptr<DatFile> _cellDat;
    unique_ptr<DatFile> _highresDat;
    unique_ptr<Camera> _camera;
    // obviously we'll want more than one at some point
    unique_ptr<Landblock> _landblock;
#ifndef HEADLESS
    unique_ptr<Renderer> _renderer;
#endif
};

#endif
