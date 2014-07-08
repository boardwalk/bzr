#include "DatFile.hpp"
#include <SDL.h>
#include <SDL_opengl.h>
#include <iostream>
#include <cstdlib>

#include "VertexShader.h"
#include "FragmentShader.h"

#define CHECK_GL(stmt) { stmt; CheckGL(); }

class GLError : public runtime_error
{
public:
    GLError(const string& msg) : runtime_error(msg) { }
};

static void CheckGL()
{
    GLenum e = glGetError();

    if(e != GL_NO_ERROR)
    {
        char buf[16];
        sprintf(buf, "%08x", e);

        throw GLError(buf);
    }
}

class Renderer
{
public:
    void init();
    void cleanup();
    void render();

private:
    void initProgram();
    void cleanupProgram();
};

void Renderer::init()
{
    initProgram();
}

void Renderer::cleanup()
{
    cleanupProgram();
}

void Renderer::render()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

static GLuint initShader(GLenum type, const GLchar* source, const GLint length)
{
    GLuint shader;
    CHECK_GL(shader = glCreateShader(type));
    CHECK_GL(glShaderSource(shader, 1, &source, &length));
    CHECK_GL(glCompileShader(shader));

    GLint success = GL_FALSE;
    CHECK_GL(glGetShaderiv(shader, GL_COMPILE_STATUS, &success));

    if(!success)
    {
        GLint logLength;
        CHECK_GL(glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength));

        vector<GLchar> log(logLength);
        CHECK_GL(glGetShaderInfoLog(shader, logLength, &logLength, log.data()));

        string logStr(log.data(), logLength);
        throw GLError(logStr);
    }

    return shader;
}

void Renderer::initProgram()
{
    GLuint vertexShader = initShader(GL_VERTEX_SHADER, VertexShader, sizeof(VertexShader)/sizeof(VertexShader[0]));
    GLuint fragmentShader = initShader(GL_FRAGMENT_SHADER, FragmentShader, sizeof(FragmentShader)/sizeof(FragmentShader[0]));

    GLuint program;
    CHECK_GL(program = glCreateProgram());
    CHECK_GL(glAttachShader(program, vertexShader));
    CHECK_GL(glAttachShader(program, fragmentShader));
    CHECK_GL(glLinkProgram(program));

    GLint success = GL_FALSE;
    CHECK_GL(glGetProgramiv(program, GL_LINK_STATUS, &success));

    if(!success)
    {
        GLint logLength;
        CHECK_GL(glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength));

        vector<GLchar> log(logLength);
        CHECK_GL(glGetProgramInfoLog(program, logLength, &logLength, log.data()));

        string logStr(log.data(), logLength);
        throw GLError(logStr);
    }

    CHECK_GL(glUseProgram(program));
    CHECK_GL(glDeleteShader(vertexShader));
    CHECK_GL(glDeleteShader(fragmentShader));
    CHECK_GL(glDeleteProgram(program));
}

void Renderer::cleanupProgram()
{
    CHECK_GL(glUseProgram(0));
}

int main()
{
    try
    {
        SDL_Init(SDL_INIT_VIDEO);

        SDL_Window* window = SDL_CreateWindow("Bael'Zharon's Respite",
            SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 800, 600, SDL_WINDOW_OPENGL);

        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
        SDL_GLContext context = SDL_GL_CreateContext(window);

        Renderer renderer;
        renderer.init();

        bool running = true;

        while(running)
        {
            SDL_Event event;

            while(SDL_PollEvent(&event) != 0)
            {
                switch(event.type)
                {
                    case SDL_QUIT:
                        running = false;
                        break;
                }
            }

            renderer.render();
            SDL_GL_SwapWindow(window);
        }

        renderer.cleanup();

        SDL_GL_DeleteContext(context);
        SDL_DestroyWindow(window);
        SDL_Quit();
    }
    catch(const runtime_error& e)
    {
        cerr << "An error ocurred: " << e.what() << endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
