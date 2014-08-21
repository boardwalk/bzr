#include "graphics/LandblockRenderer.h"
#include "graphics/util.h"
#include "math/Mat3.h"
#include "Camera.h"
#include "Core.h"
#include "LandblockManager.h"
#include "ResourceCache.h"
#include "Texture.h"
#include <algorithm>

#include "graphics/shaders/LandVertexShader.h"
#include "graphics/shaders/LandTessControlShader.h"
#include "graphics/shaders/LandTessEvalShader.h"
#include "graphics/shaders/LandFragmentShader.h"

static const uint32_t LANDSCAPE_TEXTURES[] =
{
    0x06006d6f, // 0x00 BarrenRock
    0x06006d49, // 0x01 Grassland
    0x00000000, // 0x02 Ice
    0x06006d06, // 0x03 LushGrass
    0x00000000, // 0x04 MarshSparseSwamp
    0x00000000, // 0x05 MudRichDirt
    0x00000000, // 0x06 ObsidianPlain
    0x06006d46, // 0x07 PackedDirt
    0x00000000, // 0x08 PatchyDirtFx
    0x06006d3c, // 0x09 PatchyGrassland
    0x00000000, // 0x0A sand-yellow
    0x06006d44, // 0x0B sand-grey
    0x00000000, // 0x0C sand-rockStrewn
    0x00000000, // 0x0D SedimentaryRock
    0x06006d41, // 0x0E SemiBarrenRock
    0x00000000, // 0x0F Snow
    0x06006d45, // 0x10 WaterRunning
    0x00000000, // 0x11 WaterStandingFresh
    0x06006d4f, // 0x12 WaterShallowSea
    0x00000000, // 0x13 WaterShallowStillSea
    0x06006d4e, // 0x14 WaterDeepSea
    0x06006d40, // 0x15 forestfloor
    0x00000000, // 0x16 FauxWaterRunning
    0x00000000, // 0x17 SeaSlime
    0x00000000, // 0x18 Agiland
    0x00000000, // 0x19 Volcano1
    0x00000000, // 0x1A Volcano2
    0x00000000, // 0x1B BlueIce
    0x00000000, // 0x1C Moss
    0x00000000, // 0x1D DarkMoss
    0x00000000, // 0x1E olthoi
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
    0x06006d60, // 9 big corner
    0x06006d36  // A wavy diagonal
};

static const int BLEND_ARRAY_SIZE = 512;
static const int BLEND_ARRAY_DEPTH = sizeof(BLEND_TEXTURES) / sizeof(BLEND_TEXTURES[0]);

LandblockRenderer::LandblockRenderer()
{
    initProgram();
    initTerrainTexture();
    initBlendTexture();
}

LandblockRenderer::~LandblockRenderer()
{
    _program.destroy();
    glDeleteTextures(1, &_terrainTexture);
    glDeleteTextures(1, &_blendTexture);
}

void LandblockRenderer::render(const Mat4& projectionMat, const Mat4& viewMat)
{
    _program.use();

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D_ARRAY, _terrainTexture);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D_ARRAY, _blendTexture);

    auto& landblockManager = Core::get().landblockManager();

    auto cameraPosition = Core::get().camera().position();
    glUniform4f(_program.getUniform("cameraPosition"), GLfloat(cameraPosition.x), GLfloat(cameraPosition.y), GLfloat(cameraPosition.z), 1.0f);

    auto viewLightPosition = viewMat * _lightPosition;
    glUniform3f(_program.getUniform("lightPosition"), GLfloat(viewLightPosition.x), GLfloat(viewLightPosition.y), GLfloat(viewLightPosition.z));

    for(auto& pair : landblockManager)
    {
        auto dx = pair.first.x() - landblockManager.center().x();
        auto dy = pair.first.y() - landblockManager.center().y();

        Mat4 modelMat;
        modelMat.makeTranslation(Vec3(dx * 192.0, dy * 192.0, 0.0));

        auto modelViewMat = viewMat * modelMat;
        auto modelViewProjectionMat = projectionMat * modelViewMat;

        loadMat3ToUniform(Mat3(modelViewMat).inverse().transpose(), _program.getUniform("normalMatrix"));
        loadMat4ToUniform(modelMat, _program.getUniform("modelMatrix"));
        loadMat4ToUniform(modelViewMat, _program.getUniform("modelViewMatrix"));
        loadMat4ToUniform(modelViewProjectionMat, _program.getUniform("modelViewProjectionMatrix"));

        auto& renderData = pair.second.renderData();

        if(!renderData)
        {
            renderData.reset(new LandblockRenderData(pair.second));
        }

        auto& landblockRenderData = *(LandblockRenderData*)renderData.get();

        landblockRenderData.bind(_program);

        glDrawArrays(GL_PATCHES, 0, landblockRenderData.vertexCount());
    }
}

void LandblockRenderer::setLightPosition(const Vec3& lightPosition)
{
    _lightPosition = lightPosition;
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
    glUniform1i(terrainTexLocation, 0); // corresponds to GL_TEXTURE0

    auto blendTexLocation = _program.getUniform("blendTex");
    glUniform1i(blendTexLocation, 1);

    auto offsetTexLocation = _program.getUniform("offsetTex");
    glUniform1i(offsetTexLocation, 2);

    auto normalTexLocation = _program.getUniform("normalTex");
    glUniform1i(normalTexLocation, 3);

    // lighting parameters
    glUniform3f(_program.getUniform("lightIntensity"), 1.0f, 1.0f, 1.0f);
    glUniform3f(_program.getUniform("Kd"), 0.7f, 0.7f, 0.7f);
    glUniform3f(_program.getUniform("Ka"), 0.5f, 0.5f, 0.5f);
    glUniform3f(_program.getUniform("Ks"), 0.0f, 0.0f, 0.0f);
    glUniform1f(_program.getUniform("shininess"), 1.0);
}

void LandblockRenderer::initTerrainTexture()
{
    // allocate terrain texture
    glGenTextures(1, &_terrainTexture);
    glBindTexture(GL_TEXTURE_2D_ARRAY, _terrainTexture);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGB8, TERRAIN_ARRAY_SIZE, TERRAIN_ARRAY_SIZE, TERRAIN_ARRAY_DEPTH, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);

    // populate terrain texture 
    for(auto i = 0; i < TERRAIN_ARRAY_DEPTH; i++)
    {
        Image image;

        if(LANDSCAPE_TEXTURES[i] == 0x00000000)
        {
            image.init(ImageFormat::RGB24, TERRAIN_ARRAY_SIZE, TERRAIN_ARRAY_SIZE, nullptr);
            image.fill(0xFF);
        }
        else
        {
            auto texture = Core::get().resourceCache().get(LANDSCAPE_TEXTURES[i]);
            image = texture->cast<Texture>().image();
            image.scale(TERRAIN_ARRAY_SIZE, TERRAIN_ARRAY_SIZE);
        }

        GLenum format;

        if(image.format() == ImageFormat::RGB24)
        {
            format = GL_RGB;
        }
        else if(image.format() == ImageFormat::BGR24)
        {
            format = GL_BGR;
        }
        else if(image.format() == ImageFormat::BGRA32)
        {
            format = GL_BGRA;
        }
        else
        {
            throw runtime_error("Bad terrain image format");
        }

        glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, i, TERRAIN_ARRAY_SIZE, TERRAIN_ARRAY_SIZE, 1, format, GL_UNSIGNED_BYTE, image.data());
    }

    glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
}

void LandblockRenderer::initBlendTexture()
{
    // allocate terrain texture
    glGenTextures(1, &_blendTexture);
    glBindTexture(GL_TEXTURE_2D_ARRAY, _blendTexture);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_R8, BLEND_ARRAY_SIZE, BLEND_ARRAY_SIZE, BLEND_ARRAY_DEPTH, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr);

    // populate terrain texture 
    for(auto i = 0; i < BLEND_ARRAY_DEPTH; i++)
    {
        Image image;

        if(BLEND_TEXTURES[i] == 0x00000000)
        {
            image.init(ImageFormat::A8, BLEND_ARRAY_SIZE, BLEND_ARRAY_SIZE, nullptr);
        }
        else if(BLEND_TEXTURES[i] == 0xFFFFFFFF)
        {
            image.init(ImageFormat::A8, BLEND_ARRAY_SIZE, BLEND_ARRAY_SIZE, nullptr);
            image.fill(0xFF);
        }
        else
        {
            auto texture = Core::get().resourceCache().get(BLEND_TEXTURES[i]);
            image = texture->cast<Texture>().image();
        }

        if(image.width() != BLEND_ARRAY_SIZE || image.height() != BLEND_ARRAY_SIZE)
        {
            throw runtime_error("Bad terrain image size");
        }

        if(image.format() != ImageFormat::A8)
        {
            throw runtime_error("Bad terrain image format");
        }

        glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, i, BLEND_ARRAY_SIZE, BLEND_ARRAY_SIZE, 1, GL_RED, GL_UNSIGNED_BYTE, image.data());
    }
}
