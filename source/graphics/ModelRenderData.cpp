#include "graphics/ModelRenderData.h"
#include "Model.h"
#include "Texture.h"
#include "TextureLookup5.h"
#include "TextureLookup8.h"
#include <algorithm>
#include <vector>

ModelRenderData::ModelRenderData(const Model& model)
{
    initTexture(model);
    initGeometry(model);
}

ModelRenderData::~ModelRenderData()
{
    glDeleteVertexArrays(1, &_vertexArray);
    glDeleteBuffers(1, &_vertexBuffer);
    glDeleteBuffers(1, &_indexBuffer);
    glDeleteTextures(1, &_texture);
}

void ModelRenderData::bind()
{
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D_ARRAY, _texture);

    glBindVertexArray(_vertexArray);
}

GLsizei ModelRenderData::indexCount() const
{
    return _indexCount;
}

static ResourcePtr getTexture(ResourcePtr resource)
{
    if(!resource)
    {
        return ResourcePtr();
    }

    switch(resource->resourceType())
    {
        case Resource::Texture:
            return resource;

        case Resource::TextureLookup5:
            return getTexture(resource->cast<TextureLookup5>().texture());

        case Resource::TextureLookup8:
            return getTexture(resource->cast<TextureLookup8>().texture());

        default:
            return ResourcePtr();
    }
}

// If texture of multiple formats are used in the same model, we have to choose a common format to convert to
// We prefer uncompressed formats over compressed and ones with alpha over ones without
static int formatValue(Image::Format f)
{
    switch(f)
    {
        case Image::Invalid:
            return 0;
        case Image::Paletted16:
            // we should not have a paletted texture at this point
            assert(false);
            return 0;
        case Image::A8:
            // blend textures are used in landscape and shouldn't be referenced here
            assert(false);
            return 0;
        case Image::RGB24:
            // RGB textures are used in landscape and shouldn't be referenced here
            assert(false);
            return 0;
        case Image::DXT1:
        case Image::DXT3:
        case Image::DXT5:
            return 1;
        case Image::BGR24:
            return 2;
        case Image::BGRA32:
            return 3;
        default:
            // what's this?
            assert(false);
            return 0;
    }
}

static GLenum formatInternalGLEnum(Image::Format f)
{
    switch(f)
    {
        case Image::DXT1:
            return GL_COMPRESSED_RGB_S3TC_DXT1_EXT; // Or RGBA? Hmmm.
        case Image::DXT3:
            return GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
        case Image::DXT5:
            return GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
        case Image::BGR24:
            return GL_RGB;
        case Image::BGRA32:
            return GL_RGBA;
        default:
            // what's this?
            assert(false);
            return 0;
    }
}

static GLenum formatGLEnum(Image::Format f)
{
    switch(f)
    {
        case Image::DXT1:
            return GL_COMPRESSED_RGB_S3TC_DXT1_EXT; // Or RGBA? Hmmm.
        case Image::DXT3:
            return GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
        case Image::DXT5:
            return GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
        case Image::BGR24:
            return GL_BGR;
        case Image::BGRA32:
            return GL_BGRA;
        default:
            // what's this?
            assert(false);
            return 0;
    }
}

static void scaledUpload(const Image& image, GLuint targetArrayTex, int zOffset, GLenum internalFormat, int targetWidth, int targetHeight)
{
    // upload image to sourceTex
    GLuint sourceTex;
    glGenTextures(1, &sourceTex);
    glBindTexture(GL_TEXTURE_2D, sourceTex);

    if(Image::formatIsCompressed(image.format()))
    {
        printf("uploading scaled compressed\n");
        //glCompressedTexImage2D(GL_TEXTURE_2D, 0, formatInternalGLEnum(image.format()), image.width(), image.height(), 0, (GLsizei)image.size(), image.data());
    }
    else
    {
        printf("uploading scaled uncompressed\n");
        //glTexImage2D(GL_TEXTURE_2D, 0, formatInternalGLEnum(image.format()), image.width(), image.height(), 0, formatGLEnum(image.format()), GL_UNSIGNED_BYTE, image.data());
    }

    // alias targetArrayTex to targetTex
    GLuint targetTex;
    (void)targetArrayTex;
    (void)internalFormat;
    (void)zOffset;
    glGenTextures(1, &targetTex);
    glTextureView(targetTex, GL_TEXTURE_2D, targetArrayTex, internalFormat, 0, 1, zOffset, 1);

    // create framebuffers for sourceTex and targetTex
    GLuint sourceFramebuffer;
    glGenFramebuffers(1, &sourceFramebuffer);
    //glBindFramebuffer(GL_FRAMEBUFFER, sourceFramebuffer);
    //glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, sourceTex, 0);

    GLuint targetFramebuffer;
    glGenFramebuffers(1, &targetFramebuffer);
    //glBindFramebuffer(GL_FRAMEBUFFER, targetFramebuffer);
    //glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, targetTex, 0);

    // blit from sourceTex to targetTex
    (void)targetWidth;
    (void)targetHeight;
    //glBindFramebuffer(GL_READ_FRAMEBUFFER, sourceFramebuffer);
    //glBindFramebuffer(GL_DRAW_FRAMEBUFFER, targetFramebuffer);
    //glBlitFramebuffer(0, 0, image.width(), image.height(), 0, 0, targetWidth, targetHeight, GL_COLOR_BUFFER_BIT, GL_LINEAR);
    //glBindFramebuffer(GL_FRAMEBUFFER, 0);
    // TODO restore old framebuffer?

    // clean up
    glDeleteTextures(1, &sourceTex);
    glDeleteTextures(1, &targetTex);
    glDeleteFramebuffers(1, &sourceFramebuffer);
    glDeleteFramebuffers(1, &targetFramebuffer);
}

static void regularUpload(const Image& image, GLuint targetArrayTex, int zOffset)
{
    glBindTexture(GL_TEXTURE_2D_ARRAY, targetArrayTex);

    if(Image::formatIsCompressed(image.format()))
    {
        printf("uploading regular compressed\n");
        glCompressedTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, zOffset, image.width(), image.height(), 1, formatGLEnum(image.format()), (GLsizei)image.size(), image.data());
    }
    else
    {
        printf("uploading regular uncompressed\n");
        glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, zOffset, image.width(), image.height(), 1, formatGLEnum(image.format()), GL_UNSIGNED_BYTE, image.data());
    }
}

void ModelRenderData::initTexture(const Model& model)
{
    // Choose common texture format and size
    Image::Format format = Image::Invalid;
    GLsizei maxWidth = 0;
    GLsizei maxHeight = 0;

    for(auto& resource : model.textures())
    {
        auto innerResource = getTexture(resource);

        if(!innerResource)
        {
            continue;
        }

        auto& image = innerResource->cast<Texture>().image();

        if(formatValue(image.format()) > formatValue(format))
        {
            format = image.format();
        }

        maxWidth = max(maxWidth, GLsizei(image.width()));
        maxHeight = max(maxHeight, GLsizei(image.height()));
    }

    if(format == Image::Invalid)
    {
        printf("WOT!\n");
        return;
    }

    glGenTextures(1, &_texture);
    glBindTexture(GL_TEXTURE_2D_ARRAY, _texture);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, formatInternalGLEnum(format), maxWidth, maxHeight, (GLsizei)model.textures().size(), 0, GL_BGRA, GL_UNSIGNED_BYTE, nullptr);   

    GLint zOffset = 0;

    for(auto& resource : model.textures())
    {
        auto innerResource = getTexture(resource);

        if(!innerResource)
        {
            continue;
        }

        auto& image = innerResource->cast<Texture>().image();

        if(image.width() != maxWidth || image.height() != maxHeight)
        {
            scaledUpload(image, _texture, zOffset, formatInternalGLEnum(format), maxWidth, maxHeight);
        }
        else
        {
            regularUpload(image, _texture, zOffset);
        }

        zOffset++;
    }

    glActiveTexture(GL_TEXTURE0);
    glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
}

void ModelRenderData::initGeometry(const Model& model)
{
    // vx, vy, vz, nx, ny, nz, s, t, p
    static const int COMPONENTS_PER_VERTEX = 9;

    // we're duplicating a bit to make all this fit the structure of modern graphics APIs
    // the models are so low resolution it should not matter

    vector<float> vertexData;
    vector<uint16_t> indexData;

    for(auto& primitive : model.primitives())
    {
        if(!indexData.empty())
        {
            indexData.push_back(0xFFFF);
        }
        
        for(auto& index : primitive.indices)
        {
            indexData.push_back(uint16_t(vertexData.size() / COMPONENTS_PER_VERTEX));

            auto& vertex = model.vertices()[index.vertexIndex];

            vertexData.push_back(float(vertex.position.x));
            vertexData.push_back(float(vertex.position.y));
            vertexData.push_back(float(vertex.position.z));

            vertexData.push_back(float(vertex.normal.x));
            vertexData.push_back(float(vertex.normal.y));
            vertexData.push_back(float(vertex.normal.z));

            if(vertex.texCoords.empty())
            {
                vertexData.push_back(0.0f);
                vertexData.push_back(1.0f);
            }
            else
            {
                vertexData.push_back(float(vertex.texCoords[index.texCoordIndex].x));
                vertexData.push_back(1.0f - float(vertex.texCoords[index.texCoordIndex].y));
            }

            if(primitive.texIndex == 0xFFFF)
            {
                vertexData.push_back(0.0f);
            }
            else
            {
                vertexData.push_back(float(primitive.texIndex));
            }
        }
    }

    _indexCount = GLsizei(indexData.size());

    glGenVertexArrays(1, &_vertexArray);
    glBindVertexArray(_vertexArray);

    glGenBuffers(1, &_vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, _vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(float), vertexData.data(), GL_STATIC_DRAW);

    glGenBuffers(1, &_indexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _indexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexData.size() * sizeof(uint16_t), indexData.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * COMPONENTS_PER_VERTEX, nullptr);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float) * COMPONENTS_PER_VERTEX, (GLvoid*)(sizeof(float) * 3));
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(float) * COMPONENTS_PER_VERTEX, (GLvoid*)(sizeof(float) * 6));

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
}
