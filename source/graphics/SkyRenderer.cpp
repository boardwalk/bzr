#include "graphics\SkyRenderer.h"
#include "graphics\util.h"
#include "math\Mat4.h"
#include <vector>

#include "graphics/shaders/SkyVertexShader.glsl.h"
#include "graphics/shaders/SkyFragmentShader.glsl.h"

SkyRenderer::SkyRenderer()
{
    initProgram();
    initGeometry();
    initTexture();
}

SkyRenderer::~SkyRenderer()
{
    _program.destroy();
    glDeleteVertexArrays(1, &_vertexArray);
    glDeleteBuffers(1, &_vertexBuffer);
    glDeleteTextures(1, &_texture);
}

void SkyRenderer::render(const Mat4& projMat, const Mat4& viewMat)
{
    _program.use();

    loadMat4ToUniform(projMat * viewMat, _program.getUniform("modelViewProjectionMatrix"));

    glBindVertexArray(_vertexArray);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D_ARRAY, _texture);

    //glDisable(GL_DEPTH_TEST);

    glDrawArrays(GL_TRIANGLES, 0, _vertexCount);

    //glEnable(GL_DEPTH_TEST);
}

void SkyRenderer::initProgram()
{
    _program.create();
    _program.attach(GL_VERTEX_SHADER, SkyVertexShader);
    _program.attach(GL_FRAGMENT_SHADER, SkyFragmentShader);
    _program.link();
}

void SkyRenderer::initGeometry()
{
    static float VERTEX_DATA[] =
    {
        // -x face
        -0.5, -0.5, -0.5,
        -0.5, +0.5, -0.5,
        -0.5, -0.5, +0.5,

        -0.5, +0.5, +0.5,
        -0.5, -0.5, +0.5,
        -0.5, +0.5, -0.5,

        // +x face
        +0.5, +0.5, -0.5,
        +0.5, -0.5, -0.5,
        +0.5, +0.5, +0.5,

        +0.5, -0.5, +0.5,
        +0.5, +0.5, +0.5,
        +0.5, -0.5, -0.5,

        // -y face
        +0.5, -0.5, -0.5,
        -0.5, -0.5, -0.5,
        +0.5, -0.5, +0.5,

        -0.5, -0.5, +0.5,
        +0.5, -0.5, +0.5,
        -0.5, -0.5, -0.5,

        // +y face
        -0.5, +0.5, -0.5,
        +0.5, +0.5, -0.5,
        -0.5, +0.5, +0.5,

        +0.5, +0.5, +0.5,
        +0.5, +0.5, -0.5,
        -0.5, +0.5, +0.5,

        // -z face
        -0.5, -0.5, -0.5,
        +0.5, -0.5, -0.5,
        -0.5, +0.5, -0.5,

        +0.5, +0.5, -0.5,
        -0.5, +0.5, -0.5,
        +0.5, -0.5, -0.5,

        // +z face
        -0.5, +0.5, +0.5,
        +0.5, +0.5, +0.5,
        -0.5, -0.5, +0.5,

        +0.5, -0.5, +0.5,
        -0.5, -0.5, +0.5,
        +0.5, +0.5, +0.5
    };

    static const int COMPONENTS_PER_VERTEX = 3;

    _vertexCount = sizeof(VERTEX_DATA) / sizeof(VERTEX_DATA[0]) / COMPONENTS_PER_VERTEX;

    glGenVertexArrays(1, &_vertexArray);
    glBindVertexArray(_vertexArray);

    glGenBuffers(1, &_vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, _vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(VERTEX_DATA), VERTEX_DATA, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * COMPONENTS_PER_VERTEX, nullptr);

    glEnableVertexAttribArray(0);
}

void SkyRenderer::initTexture()
{
    static const GLenum FACES[] =
    {
        GL_TEXTURE_CUBE_MAP_POSITIVE_X,
        GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
        GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
        GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
        GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
        GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
    };

    static const int NUM_FACES = sizeof(FACES) / sizeof(FACES[0]);

    glGenTextures(1, &_texture);
    glBindTexture(GL_TEXTURE_CUBE_MAP, _texture);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    for(int i = 0; i < NUM_FACES; i++)
    {
        vector<uint8_t> data(128 * 128 * 3, double(i) / sizeof(NUM_FACES) * 0xA0 + 0x5F);
        glTexImage2D(FACES[i], 0, GL_RGB8, 128, 128, 0, GL_RGB, GL_UNSIGNED_BYTE, data.data());
    }
}

