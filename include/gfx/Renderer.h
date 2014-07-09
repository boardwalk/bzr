#ifndef BZR_RENDERER_H
#define BZR_RENDERER_H

#include "Noncopyable.h"

#include <SDL.h>
#define GL_GLEXT_PROTOTYPES
#include <SDL_opengl.h>

// CHECK_GL throws an exception if stmt generates a glGetError code
#ifndef NDEBUG
#define CHECK_GL(stmt) stmt
#else
#define CHECK_GL(stmt) { stmt; checkGL(); }
void checkGL();
#endif

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
