#ifndef BZR_GRAPHICS_RENDERER_H
#define BZR_GRAPHICS_RENDERER_H

#include "Noncopyable.h"
#include "graphics/Program.h"

class Renderer : Noncopyable
{
public:
    Renderer();
    ~Renderer();

    void render(double interp);

private:
	void initFramebuffer(int width, int height);
	void cleanupFramebuffer();

    bool _videoInit;
    SDL_Window* _window;
    SDL_GLContext _context;

    Program _program;

    GLuint _framebuffer;
    GLuint _colorTexture;
    GLuint _normalTexture;
    GLuint _depthTexture;
};

#endif
