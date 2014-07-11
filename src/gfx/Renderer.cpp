#include "gfx/Renderer.h"
#include "gfx/LandblockRenderer.h"
#include "gfx/util.h"
#include "Game.h"
#include "Camera.h"
#include "Landblock.h"
#include "util.h"
#include <SDL.h>
#include <string>
#include <vector>

#include "gfx/shaders/VertexShader.glsl.h"
#include "gfx/shaders/FragmentShader.glsl.h"

static const GLfloat PI = 3.14159265359;

static const GLfloat vertices[] =
{
    0.0f, 1.0f, 3.0f,
    -1.0f, -1.0f, 3.0f,
    1.0f, -1.0f, 3.0f
};

void identityMatrix(GLfloat mat[16])
{
    memset(mat, 0, sizeof(GLfloat) * 16);

    mat[0] = 1.0f;
    mat[5] = 1.0f;
    mat[10] = 1.0f;
    mat[15] = 1.0f;
}

//
// on coordinates
// our coordinate system is:
// +x goes right
// +y goes up
// +z goes out of the screen
// this is "right handed"
// http://www.songho.ca/opengl/gl_projectionmatrix.html
//
void perspectiveMatrix(GLfloat fovy, GLfloat aspect, GLfloat zNear, GLfloat zFar, GLfloat m[16])
{
   memset(m, 0, sizeof(GLfloat) * 16);

   auto f = 1.0f / tan(fovy * PI / 360.0f);
   m[0] = f / aspect;
   m[5] = f;
   m[10] = -(zFar + zNear) / (zFar - zNear);
   m[11] = -1.0f;
   m[14] = -(2.0f * zFar * zNear) / (zFar - zNear);
}

static GLuint createShader(GLenum type, const GLchar* source)
{
    GLint length = strlen(source);

    auto shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, &length);
    glCompileShader(shader);

    GLint success = GL_FALSE;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

    if(!success)
    {
        GLint logLength;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);

        vector<GLchar> log(logLength);
        glGetShaderInfoLog(shader, logLength, &logLength, log.data());

        string logStr(log.data(), logLength);
        throw runtime_error(logStr);
    }

    return shader;
}

static GLuint createProgram(const GLchar* vertexSource, const GLchar* fragmentSource)
{
    auto vertexShader = createShader(GL_VERTEX_SHADER, VertexShader);
    auto fragmentShader = createShader(GL_FRAGMENT_SHADER, FragmentShader);

    auto program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);

    GLint success = GL_FALSE;
    glGetProgramiv(program, GL_LINK_STATUS, &success);

    if(!success)
    {
        GLint logLength;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);

        vector<GLchar> log(logLength);
        glGetProgramInfoLog(program, logLength, &logLength, log.data());

        string logStr(log.data(), logLength);
        throw runtime_error(logStr);
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return program;
}

Renderer::Renderer() : _videoInit(false), _window(nullptr), _context(nullptr)
{
    if(SDL_InitSubSystem(SDL_INIT_VIDEO) < 0)
    {
        throwSDLError();
    }

    _videoInit = true;

#ifdef __APPLE__
    // Apple's drivers don't support the compatibility profile on GL >v2.1
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
#endif

    _window = SDL_CreateWindow("Bael'Zharon's Respite",
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 800, 600, SDL_WINDOW_OPENGL);

    if(_window == nullptr)
    {
        throwSDLError();
    }

    _context = SDL_GL_CreateContext(_window);

    if(_context == nullptr)
    {
        throwSDLError();
    }

#ifdef _MSC_VER
	auto glewErr = glewInit();

	if(glewErr != GLEW_OK)
	{
		throw runtime_error(string("Unable to initialize GLEW: ") + (const char*)glewGetErrorString(glewErr));
	}
#endif

    SDL_GL_SetSwapInterval(1); // vsync
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.0f, 0.0, 0.5f, 1.0f);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    glEnable(GL_PRIMITIVE_RESTART);
    glPrimitiveRestartIndex(0xffff);

    _program = createProgram(VertexShader, FragmentShader);
    glUseProgram(_program);

    GLfloat projectionMat[16];
    perspectiveMatrix(90.0f, 800.0/600.0f, 0.1f, 1000.0f, projectionMat);

    //GLfloat viewMat[16];
    //identityMatrix(viewMat);

    GLfloat modelMat[16];
    identityMatrix(modelMat);

    auto projectionLoc = glGetUniformLocation(_program, "projection");
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, projectionMat);

    //auto viewLoc = glGetUniformLocation(_program, "view");
    //glUniformMatrix4fv(viewLoc, 1, GL_FALSE, viewMat);

    auto modelLoc = glGetUniformLocation(_program, "model");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, modelMat);

    GLuint vertexArray;
    glGenVertexArrays(1, &vertexArray);
    glBindVertexArray(vertexArray);

    //glGenBuffers(1, &_buffer);
    //glBindBuffer(GL_ARRAY_BUFFER, _buffer);
    //glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
}

Renderer::~Renderer()
{
    // TODO delete VAO
    glUseProgram(0);
    //glBindBuffer(GL_ARRAY_BUFFER, 0);

    glDeleteProgram(_program);
    //glDeleteBuffers(1, & _buffer);

    if(_context != nullptr)
    {
        SDL_GL_DeleteContext(_context);
    }

    if(_window != nullptr)
    {
        SDL_DestroyWindow(_window);
    }

    if(_videoInit)
    {
        SDL_QuitSubSystem(SDL_INIT_VIDEO);
    }
}

void Renderer::render(Game& game, double interp)
{
    auto viewLoc = glGetUniformLocation(_program, "view");
    loadMat4ToUniform(game.camera().viewMatrix(), viewLoc);

    //glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    //glEnableVertexAttribArray(0);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    //glDrawArrays(GL_TRIANGLES, 0, sizeof(vertices) / sizeof(vertices[0]) / 3);

    auto& landblock = game.landblock();
    auto& renderData = landblock.renderData();

    if(!renderData)
    {
        renderData.reset(new LandblockRenderer(landblock));
    }

    auto& landblockRenderer = (LandblockRenderer&)*renderData;
    landblockRenderer.render();

    SDL_GL_SwapWindow(_window);
}

