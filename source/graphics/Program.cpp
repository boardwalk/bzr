#include "graphics/Program.h"
#include <string>
#include <vector>

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

Program::Program() : _handle(0)
{}

Program::Program(Program&& other)
{
    _handle = other._handle;
    other._handle = 0;
}

Program::~Program()
{
    destroy();
}

Program& Program::operator=(Program&& other)
{
    destroy();
    _handle = other._handle;
    other._handle = 0;
    return *this;
}

void Program::create(const char* vertexShader, const char* fragmentShader)
{
    auto vertexShaderHandle = createShader(GL_VERTEX_SHADER, vertexShader);
    auto fragmentShaderHandle = createShader(GL_FRAGMENT_SHADER, fragmentShader);

    _handle = glCreateProgram();
    glAttachShader(_handle, vertexShaderHandle);
    glAttachShader(_handle, fragmentShaderHandle);
    glLinkProgram(_handle);

    glDeleteShader(vertexShaderHandle);
    glDeleteShader(fragmentShaderHandle);

    GLint success = GL_FALSE;
    glGetProgramiv(_handle, GL_LINK_STATUS, &success);

    if(!success)
    {
        GLint logLength;
        glGetProgramiv(_handle, GL_INFO_LOG_LENGTH, &logLength);

        vector<GLchar> log(logLength);
        glGetProgramInfoLog(_handle, logLength, &logLength, log.data());

        string logStr(log.data(), logLength);
        throw runtime_error(logStr);
    }    
}

void Program::destroy()
{
    if(_handle != 0)
    {
        glDeleteProgram(_handle);
        _handle = 0;
    }
}

void Program::use()
{
    assert(_handle != 0);
    glUseProgram(_handle);
}

GLint Program::getUniform(const char* name)
{
    assert(_handle != 0);
    return glGetUniformLocation(_handle, name);
}
