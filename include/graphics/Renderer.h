#ifndef BZR_GRAPHICS_RENDERER_H
#define BZR_GRAPHICS_RENDERER_H

#include "graphics/Program.h"
#include "Noncopyable.h"

class Renderer : Noncopyable
{
public:
    Renderer();
    ~Renderer();

    void render(double interp);

private:
    int _width;
    int _height;
    double _fieldOfView;

    bool _videoInit;
    SDL_Window* _window;
    SDL_GLContext _context;
};

#endif
