#ifndef BZR_GAME_H
#define BZR_GAME_H

#include "Noncopyable.h"

class Renderer;
class DatFile;

// These are really just stubs and will need to be fleshed out
class Position
{

};

class GameObject
{
    const Position& getPosition() const;
};

class Game : Noncopyable
{
public:
    Game();
    ~Game();

    void run();

    const DatFile& portalDat() const;
    const DatFile& cellDat() const;

private:
    void handleEvents();
    void step(double dt);

    bool _done;
    unique_ptr<DatFile> _portalDat;
    unique_ptr<DatFile> _cellDat;
#ifndef HEADLESS
    unique_ptr<Renderer> _renderer;
#endif
};

#endif
