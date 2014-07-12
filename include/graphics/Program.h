#ifndef BZR_GRAPHICS_PROGRAM_H
#define BZR_GRAPHICS_PROGRAM_H

class Program
{
public:
    Program();
    Program(Program&& other);
    ~Program();
    Program& operator=(Program&& other);

    void create(const GLchar* vertexShader, const GLchar* fragmentShader);
    void destroy();
    
    void use();
    GLint getUniform(const GLchar* name);

private:
    GLuint _handle;
};

#endif