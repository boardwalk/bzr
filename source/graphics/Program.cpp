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

void Program::create(const GLchar* vertexShader, const GLchar* fragmentShader)
{    
    auto vertexShaderHandle = createShader(GL_VERTEX_SHADER, vertexShader);
    auto fragmentShaderHandle = createShader(GL_FRAGMENT_SHADER, fragmentShader);

    auto program = glCreateProgram();
    glAttachShader(program, vertexShaderHandle);
    glAttachShader(program, fragmentShaderHandle);
    glLinkProgram(program);

    glDeleteShader(vertexShaderHandle);
    glDeleteShader(fragmentShaderHandle);

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

    destroy();
    _handle = program;
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

GLint Program::getUniform(const GLchar* name)
{
    assert(_handle != 0);

    auto loc = glGetUniformLocation(_handle, name);

    if(loc < 0)
    {
        //throw runtime_error("Uniform does not exist");
    }

    return loc;
}
