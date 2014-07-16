#include "graphics/LandblockRenderer.h"
#include "graphics/Image.h"
#include "graphics/util.h"
#include "math/Mat3.h"
#include "math/Mat4.h"
#include "math/Vec3.h"
#include "Landblock.h"
#include <algorithm>
#include <vector>

#include "graphics/shaders/LandVertexShader.glsl.h"
#include "graphics/shaders/LandFragmentShader.glsl.h"
#include "graphics/shaders/LandTessControlShader.glsl.h"
#include "graphics/shaders/LandTessEvalShader.glsl.h"

// TODO We could only create one indexBuffer per subdivision level
// TODO We could generate the x and y of the vertex data in a shader

static const uint32_t LANDSCAPE_TEXTURES[] =
{
    0x00000000, // 0x00
    0x00000000, // 0x01
    0x00000000, // 0x02
    0x06003794, // 0x03
    0x00000000, // 0x04
    0x00000000, // 0x05
    0x00000000, // 0x06
    0x00000000, // 0x07
    0x00000000, // 0x08
    0x00000000, // 0x09
    0x00000000, // 0x0A
    0x00000000, // 0x0B
    0x00000000, // 0x0C
    0x00000000, // 0x0D
    0x00000000, // 0x0E
    0x00000000, // 0x0F
    0x00000000, // 0x10
    0x00000000, // 0x11
    0x00000000, // 0x12
    0x00000000, // 0x13
    0x00000000, // 0x14 
    0x0600379a, // 0x15
    0x00000000, // 0x16
    0x00000000, // 0x17
    0x00000000, // 0x18
    0x00000000, // 0x19
    0x00000000, // 0x1A
    0x00000000, // 0x1B
    0x00000000, // 0x1C
    0x00000000, // 0x1D
    0x00000000, // 0x1E
    0x00000000, // 0x1F
    // road textures below this line
    0x06006d3f  // 0x20
};

// 6d3e
// 6d49
// 6d51
// 6d06
// 6d3d
// 6d3f
// 6d48
// 6d46
// 6d42
// 6d41
// 6d6f
// 6d55
// 6d40
// 6d53
// 6d44
// 6d3c
// 6d50
// 6d4c
// 6d45
// 6bb5
// 6f48
// 6d54
// 6d56
// 6d4b
// 6d43
// 6810
// 6844
// 74d8
// 6d4d
// 680e
// 6d4f
// 6d4e
// 6d6a
// 6d4a
// 6b9c
// 6d47
// ----
// 74d9
// 3824
// 72b3
// 3835
// 382c
// 3828
// 382a
// 72b2
// 3821
// ....


static const int TERRAIN_ARRAY_SIZE = 512;
static const int TERRAIN_ARRAY_DEPTH = sizeof(LANDSCAPE_TEXTURES) / sizeof(LANDSCAPE_TEXTURES[0]);

static const uint32_t BLEND_TEXTURES[] =
{
    0xFFFFFFFF, // 0 special case, all white
    0x00000000, // 1 special case, all black
    0x06006d61, // 2 vertical, black to white, left of center
    0x06006d6c, // 3 top left corner, black, semi ragged
    0x06006d6d, // 4 top left corner, black, ragged
    0x06006d60, // 5 top left corner, black, rounded
    0x06006d30, // 6 vertical, black to white, very left of center, wavy
    0x06006d37, // 7 small corner
    0x06006d6b, // 8 big corner
    0x06006d60, // 9 ??
    0x06006d36  // A wavy diagonal
};

static const int BLEND_ARRAY_SIZE = 512;
static const int BLEND_ARRAY_DEPTH = sizeof(BLEND_TEXTURES) / sizeof(BLEND_TEXTURES[0]);

LandblockRenderer::LandblockRenderer(const Landblock& landblock)
{
    initProgram();
    initVAO(landblock);
    initTerrainTexture();
    initBlendTexture();
    initHeightTexture(landblock);
}

LandblockRenderer::~LandblockRenderer()
{
    _program.destroy();
    glDeleteVertexArrays(1, &_vertexArray);
    glDeleteBuffers(1, &_vertexBuffer);
    glDeleteTextures(1, &_terrainTexture);
    glDeleteTextures(1, &_blendTexture);
    glDeleteTextures(1, &_heightTexture);
}

void LandblockRenderer::render(const Mat4& projection, const Mat4& modelView)
{
    _program.use();

    glBindVertexArray(_vertexArray);

    loadMat3ToUniform(Mat3(modelView).inverse().transpose(), _program.getUniform("normalMatrix"));
    loadMat4ToUniform(modelView, _program.getUniform("modelViewMatrix"));
    loadMat4ToUniform(projection * modelView, _program.getUniform("modelViewProjectionMatrix"));

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D_ARRAY, _terrainTexture);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D_ARRAY, _blendTexture);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, _heightTexture);

    glDrawArrays(GL_PATCHES, 0, _vertexCount);
}

void LandblockRenderer::initVAO(const Landblock& landblock)
{
    auto& data = landblock.getRawData();

    vector<uint8_t> vertexData;

    for(auto y = 0; y < Landblock::GRID_SIZE - 1; y++)
    {
        for(auto x = 0; x < Landblock::GRID_SIZE - 1; x++)
        {
            //   N
            //  4-3
            //W | | E
            //  1-2
            //   S

#define T(dx, dy) (data.styles[x + (dx)][y + (dy)] >> 2) & 0x1F
            // terrain types
            auto t1 = T(0, 0);
            auto t2 = T(1, 0);
            auto t3 = T(1, 1);
            auto t4 = T(0, 1);
#undef T

#define R(dx, dy) (data.styles[x + (dx)][y + (dy)] & 0x3) != 0
            // roads
            auto r1 = R(0, 0);
            auto r2 = R(1, 0);
            auto r3 = R(1, 1);
            auto r4 = R(0, 1);
#undef R
            auto r = ((int)r1 << 12) | ((int)r2 << 8) | ((int)r3 << 4) | (int)r4;

            // flip blend texture north/south
            auto flip = false;
            // rotate blend texture 90 degree counterclockwise
            auto rotate = false;
            // road texture number
            auto rp = 0x20;
            // blend texture number
            auto bp = 0;
            auto scale = 4;

            // TODO choose a random corner blend!

            switch(r)
            {
                case 0x0000: // all ground
                    bp = 0;
                    break;
                case 0x1111: // all road
                    bp = 1;
                    break;
                case 0x1100: // south road
                    bp = 2;
                    rotate = true;
                    break;
                case 0x0011: // north road
                    bp = 2;
                    flip = true;
                    rotate = true;
                    break;
                case 0x1001: // west road
                    bp = 2; // NONE! FIXME!
                    break;
                case 0x0110: // east road
                    bp = 2; // NONE! FIXME!
                    break;
                case 0x1000: // southwest corner
                    bp = 3;
                    break;
                case 0x0100: // southeast corner
                    bp = 3;
                    rotate = true;
                    break;
                case 0x0010: // northeast corner
                    bp = 4;
                    flip = true;
                    rotate = true;
                    break;
                case 0x0001: // northwest corner
                    bp = 4;
                    flip = true;
                    break;
                case 0x1010: // southwest/northeast diagonal
                    bp = 10; // FIXME better blend texture
                    scale = 1;
                    break;
                case 0x0101: // southeast/northwest diagonal
                    bp = 10; // FIXME better blend texture
                    flip = true;
                    scale = 1;
                    break;
                default:
                    printf("fancy nignoggin\n");
                    bp = 0;
                    break;
            }

// See LandVertexShader.glsl too see what these are
#define V(dx, dy) \
    vertexData.push_back((x + (dx)) * 24); \
    vertexData.push_back((y + (dy)) * 24); \
    vertexData.push_back(dx); \
    vertexData.push_back(dy); \
    if(rotate) { \
        vertexData.push_back(scale * (flip ? 1 - (dy) : (dy))); \
        vertexData.push_back(scale * (1 - (dx))); \
    } \
    else { \
        vertexData.push_back(scale * (dx)); \
        vertexData.push_back(scale * (flip ? 1 - (dy) : (dy))); \
    } \
    vertexData.push_back(t1); \
    vertexData.push_back(t2); \
    vertexData.push_back(t3); \
    vertexData.push_back(t4); \
    vertexData.push_back(rp); \
    vertexData.push_back(bp);

            if(landblock.splitNESW(x, y))
            {
                // lower right triangle
                V(0, 0) V(1, 0) V(1, 1)

                // top left triangle
                V(1, 1) V(0, 1) V(0, 0)
            }
            else
            {
                // lower left triangle
                V(0, 0) V(1, 0) V(0, 1)

                // top right triangle
                V(1, 0) V(1, 1) V(0, 1)
            }
#undef V
        }
    }

    static const int COMPONENTS_PER_VERTEX = 12;

    _vertexCount = vertexData.size() / COMPONENTS_PER_VERTEX;

    glGenVertexArrays(1, &_vertexArray);
    glBindVertexArray(_vertexArray);

    glGenBuffers(1, &_vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, _vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(uint8_t), vertexData.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_UNSIGNED_BYTE, GL_FALSE, sizeof(uint8_t) * COMPONENTS_PER_VERTEX, nullptr);
    glVertexAttribPointer(1, 2, GL_UNSIGNED_BYTE, GL_FALSE, sizeof(uint8_t) * COMPONENTS_PER_VERTEX, (GLvoid*)(sizeof(uint8_t) * 2));
    glVertexAttribPointer(2, 2, GL_UNSIGNED_BYTE, GL_FALSE, sizeof(uint8_t) * COMPONENTS_PER_VERTEX, (GLvoid*)(sizeof(uint8_t) * 4));
    glVertexAttribPointer(3, 4, GL_UNSIGNED_BYTE, GL_FALSE, sizeof(uint8_t) * COMPONENTS_PER_VERTEX, (GLvoid*)(sizeof(uint8_t) * 6));
    glVertexAttribPointer(4, 1, GL_UNSIGNED_BYTE, GL_FALSE, sizeof(uint8_t) * COMPONENTS_PER_VERTEX, (GLvoid*)(sizeof(uint8_t) * 10));
    glVertexAttribPointer(5, 1, GL_UNSIGNED_BYTE, GL_FALSE, sizeof(uint8_t) * COMPONENTS_PER_VERTEX, (GLvoid*)(sizeof(uint8_t) * 11));

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);
    glEnableVertexAttribArray(4);
    glEnableVertexAttribArray(5);

    glBindVertexArray(0);
}

void LandblockRenderer::initProgram()
{
    _program.create();
    _program.attach(GL_VERTEX_SHADER, LandVertexShader);
    _program.attach(GL_TESS_CONTROL_SHADER, LandTessControlShader);
    _program.attach(GL_TESS_EVALUATION_SHADER, LandTessEvalShader);
    _program.attach(GL_FRAGMENT_SHADER, LandFragmentShader);
    _program.link();

    _program.use();

    // samplers
    auto terrainTexLocation = _program.getUniform("terrainTex");
    glUniform1i(terrainTexLocation, 0); // corresponds to GL_TEXTURE_0

    auto blendTexLocation = _program.getUniform("blendTex");
    glUniform1i(blendTexLocation, 1);

    auto heightTexLocation = _program.getUniform("heightTex");
    glUniform1i(heightTexLocation, 2);

    // lighting parameters
    Vec3 lightPosition(96.0, 96.0, 50.0);
    glUniform3f(_program.getUniform("lightPosition"), lightPosition.x, lightPosition.y, lightPosition.z);

    Vec3 lightIntensity(1.0, 1.0, 1.0);
    glUniform3f(_program.getUniform("lightIntensity"), lightIntensity.x, lightIntensity.y, lightIntensity.z);

    Vec3 Kd(1.0, 1.0, 1.0);
    glUniform3f(_program.getUniform("Kd"), Kd.x, Kd.y, Kd.z);

    Vec3 Ka(0.3, 0.3, 0.3);
    glUniform3f(_program.getUniform("Ka"), Ka.x, Ka.y, Ka.z);

    Vec3 Ks(0.0, 0.0, 0.0);
    glUniform3f(_program.getUniform("Ks"), Ks.x, Ks.y, Ks.z);

    glUniform1f(_program.getUniform("shininess"), 1.0);
}

void LandblockRenderer::initTerrainTexture()
{
    // allocate terrain texture
    glGenTextures(1, &_terrainTexture);
    glBindTexture(GL_TEXTURE_2D_ARRAY, _terrainTexture);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR); // default is GL_NEAREST_MIPMAP_LINEAR
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGB8, TERRAIN_ARRAY_SIZE, TERRAIN_ARRAY_SIZE, TERRAIN_ARRAY_DEPTH, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);

    // populate terrain texture 
    for(auto i = 0; i < TERRAIN_ARRAY_DEPTH; i++)
    {
        Image image;

        if(LANDSCAPE_TEXTURES[i] == 0x00000000)
        {
            image.create(Image::RGB24, TERRAIN_ARRAY_SIZE, TERRAIN_ARRAY_SIZE);
        }
        else
        {
            image.load(LANDSCAPE_TEXTURES[i]);
            image.scale(TERRAIN_ARRAY_SIZE, TERRAIN_ARRAY_SIZE);
        }

        GLenum format;

        if(image.format() == Image::RGB24)
        {
            format = GL_RGB;
        }
        else if(image.format() == Image::BGR24)
        {
            format = GL_BGR;
        }
        else if(image.format() == Image::BGRA32)
        {
            format = GL_BGRA;
        }
        else
        {
            throw runtime_error("Bad terrain image format");
        }

        glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, i, TERRAIN_ARRAY_SIZE, TERRAIN_ARRAY_SIZE, 1, format, GL_UNSIGNED_BYTE, image.data());
    }
}

void LandblockRenderer::initBlendTexture()
{
    // allocate terrain texture
    glGenTextures(1, &_blendTexture);
    glBindTexture(GL_TEXTURE_2D_ARRAY, _blendTexture);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);  // default is GL_NEAREST_MIPMAP_LINEAR
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_R8, BLEND_ARRAY_SIZE, BLEND_ARRAY_SIZE, BLEND_ARRAY_DEPTH, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr);

    // populate terrain texture 
    for(auto i = 0; i < BLEND_ARRAY_DEPTH; i++)
    {
        Image image;

        if(BLEND_TEXTURES[i] == 0x00000000)
        {
            image.create(Image::A8, BLEND_ARRAY_SIZE, BLEND_ARRAY_SIZE);
        }
        else if(BLEND_TEXTURES[i] == 0xFFFFFFFF)
        {
            image.create(Image::A8, BLEND_ARRAY_SIZE, BLEND_ARRAY_SIZE);
            image.fill(0xFF);
        }
        else
        {
            image.load(BLEND_TEXTURES[i]);
        }

        // For both the corner and edge textures, we want to use s,t > 1 to repeat emptiness to make skinnier roads
        // This flip allows us to do this
        image.flipVertical();

        if(image.width() != BLEND_ARRAY_SIZE || image.height() != BLEND_ARRAY_SIZE)
        {
            throw runtime_error("Bad terrain image size");
        }

        if(image.format() != Image::A8)
        {
            throw runtime_error("Bad terrain image format");
        }

        glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, i, BLEND_ARRAY_SIZE, BLEND_ARRAY_SIZE, 1, GL_RED, GL_UNSIGNED_BYTE, image.data());
    }
}

void LandblockRenderer::initHeightTexture(const Landblock& landblock)
{
    glGenTextures(1, &_heightTexture);
    glBindTexture(GL_TEXTURE_2D, _heightTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R16, Landblock::HEIGHT_MAP_SIZE, Landblock::HEIGHT_MAP_SIZE, 0, GL_RED, GL_UNSIGNED_SHORT, landblock.getHeightMap());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); // default is GL_NEAREST_MIPMAP_LINEAR
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    auto heightBaseLoc = _program.getUniform("heightBase");
    glUniform1f(heightBaseLoc, landblock.getHeightMapBase());

    auto heightScaleLoc = _program.getUniform("heightScale");
    glUniform1f(heightScaleLoc, landblock.getHeightMapScale());
}
