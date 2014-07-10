#ifndef BZR_GAME_H
#define BZR_GAME_H

#include "Landblock.h"
#include "Noncopyable.h"

class Renderer;
class DatFile;

class Game : Noncopyable
{
public:
    Game();
    ~Game();

    void run();

    const DatFile& portalDat() const;
    const DatFile& cellDat() const;

    Landblock& landblock();

private:
    void handleEvents();
    void step(double dt);

    bool _done;
    unique_ptr<DatFile> _portalDat;
    unique_ptr<DatFile> _cellDat;
    // obviously we'll want more than one at some point
    unique_ptr<Landblock> _landblock;
#ifndef HEADLESS
    unique_ptr<Renderer> _renderer;
#endif
};

#endif
