#ifndef BZR_GAME_H
#define BZR_GAME_H

#include "Noncopyable.h"

class DatFile;
class Camera;
class Landblock;
class Renderer;

class Game : Noncopyable
{
public:
    Game();
    ~Game();

    void run();

    const DatFile& portalDat() const;
    const DatFile& cellDat() const;
    const Camera& camera() const;

    Landblock& landblock();

private:
    void handleEvents();
    void step(double dt);

    bool _done;
    unique_ptr<DatFile> _portalDat;
    unique_ptr<DatFile> _cellDat;
    unique_ptr<Camera> _camera;
    // obviously we'll want more than one at some point
    unique_ptr<Landblock> _landblock;
#ifndef HEADLESS
    unique_ptr<Renderer> _renderer;
#endif
};

#endif
