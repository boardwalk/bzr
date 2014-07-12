#include "graphics/util.h"
#include "math/Mat3.h"
#include "math/Mat4.h"

void loadMat3ToUniform(const Mat3& mat, GLint location)
{
    GLfloat m[9];

    for(auto i = 0; i < 9; i++)
    {
        m[i] = mat.m[i];
    }

    glUniformMatrix3fv(location, 1, GL_FALSE, m);
}

void loadMat4ToUniform(const Mat4& mat, GLint location)
{
    GLfloat m[16];

    for(auto i = 0; i < 16; i++)
    {
       m[i] = mat.m[i];
    }

    glUniformMatrix4fv(location, 1, GL_FALSE, m);
}
