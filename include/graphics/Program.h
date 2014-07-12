#ifndef BZR_GRAPHICS_PROGRAM_H
#define BZR_GRAPHICS_PROGRAM_H

#include "Noncopyable.h"

class Program
{
public:
    Program();
    Program(Program&& other);
    ~Program();
    Program& operator=(Program&& other);

    void create(const char* vertexShader, const char* fragmentShader);
    void destroy();
    
    void use();
    GLint getUniform(const char* name);

private:
    GLuint _handle;
};

#endif