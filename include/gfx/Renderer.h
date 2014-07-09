#ifndef BZR_RENDERER_H
#define BZR_RENDERER_H

#include "Noncopyable.h"

class Renderer : Noncopyable
{
public:
    Renderer();
    ~Renderer();

    void render();

private:
    bool _videoInit;
    SDL_Window* _window;
    SDL_GLContext _context;

    GLuint _program;
    GLuint _buffer;
};

#endif
