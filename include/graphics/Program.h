#ifndef BZR_GRAPHICS_PROGRAM_H
#define BZR_GRAPHICS_PROGRAM_H

class Program
{
public:
    Program();
    Program(Program&& other);
    ~Program();
    Program& operator=(Program&& other);

    void create();
    void attach(GLenum type, const GLchar* source);
    void link();
    void use();
    GLint getUniform(const GLchar* name);
    void destroy();

private:
    GLuint _handle;
};

#endif