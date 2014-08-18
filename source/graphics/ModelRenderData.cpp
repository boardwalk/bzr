#include "graphics/ModelRenderData.h"
#include "graphics/Program.h"
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
    glDeleteTextures(1, &_textures);
    glDeleteTextures(1, &_textureSizes);
}

void ModelRenderData::bind()
{
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D_ARRAY, _textures);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_1D, _textureSizes);

    glBindVertexArray(_vertexArray);
}

GLsizei ModelRenderData::indexCount() const
{
    return _indexCount;
}

// If texture of multiple formats are used in the same model, we have to choose a common format to convert to
// We prefer uncompressed formats over compressed and ones with alpha over ones without
static int formatValue(ImageFormat::Value f)
{
    switch(f)
    {
        case ImageFormat::Invalid:
            return 0;
        case ImageFormat::Paletted16:
            // we should not have a paletted texture at this point
            assert(false);
            return 0;
        case ImageFormat::A8:
            // blend textures are used in landscape and shouldn't be referenced here
            assert(false);
            return 0;
        case ImageFormat::RGB24:
            // RGB textures are used in landscape and shouldn't be referenced here
            assert(false);
            return 0;
        case ImageFormat::DXT1:
        case ImageFormat::DXT3:
        case ImageFormat::DXT5:
            return 1;
        case ImageFormat::BGR24:
            return 2;
        case ImageFormat::BGRA32:
            return 3;
        default:
            // what's this?
            assert(false);
            return 0;
    }
}

static GLenum formatInternalGLEnum(ImageFormat::Value f)
{
    switch(f)
    {
        case ImageFormat::DXT1:
            return GL_COMPRESSED_RGB_S3TC_DXT1_EXT; // Or RGBA? Hmmm.
        case ImageFormat::DXT3:
            return GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
        case ImageFormat::DXT5:
            return GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
        case ImageFormat::BGR24:
            return GL_RGB8;
        case ImageFormat::BGRA32:
            return GL_RGBA8;
        default:
            // what's this?
            assert(false);
            return 0;
    }
}

static GLenum formatGLEnum(ImageFormat::Value f)
{
    switch(f)
    {
        case ImageFormat::DXT1:
            return GL_COMPRESSED_RGB_S3TC_DXT1_EXT; // Or RGBA? Hmmm.
        case ImageFormat::DXT3:
            return GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
        case ImageFormat::DXT5:
            return GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
        case ImageFormat::BGR24:
            return GL_BGR;
        case ImageFormat::BGRA32:
            return GL_BGRA;
        default:
            // what's this?
            assert(false);
            return 0;
    }
}

void ModelRenderData::initTexture(const Model& model)
{
    // Choose common texture format and size
    auto format = ImageFormat::Invalid;
    GLsizei maxWidth = 0;
    GLsizei maxHeight = 0;

    for(auto& resource : model.textures())
    {
        auto& image = resource->cast<TextureLookup8>().textureLookup5().texture().image();

        if(formatValue(image.format()) > formatValue(format))
        {
            format = image.format();
        }

        maxWidth = max(maxWidth, GLsizei(image.width()));
        maxHeight = max(maxHeight, GLsizei(image.height()));
    }

    glGenTextures(1, &_textures);
    glBindTexture(GL_TEXTURE_2D_ARRAY, _textures);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, formatInternalGLEnum(format), maxWidth, maxHeight, (GLsizei)model.textures().size(), 0, GL_BGRA, GL_UNSIGNED_BYTE, nullptr);

    vector<float> textureSizesData;
    GLint zOffset = 0;

    for(auto& resource : model.textures())
    {
        auto& image = resource->cast<TextureLookup8>().textureLookup5().texture().image();

        if(ImageFormat::isCompressed(image.format()) && !ImageFormat::isCompressed(format))
        {
            // we can't upload compressed data to an uncompressed texture
            auto uncompImage = image;
            uncompImage.decompress();

            glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, zOffset, uncompImage.width(), uncompImage.height(), 1, formatGLEnum(uncompImage.format()), GL_UNSIGNED_BYTE, uncompImage.data());
        }
        else if(ImageFormat::isCompressed(image.format()))
        {
            glCompressedTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, zOffset, image.width(), image.height(), 1, formatGLEnum(image.format()), (GLsizei)image.size(), image.data());
        }
        else
        {
            glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, zOffset, image.width(), image.height(), 1, formatGLEnum(image.format()), GL_UNSIGNED_BYTE, image.data());
        }

        textureSizesData.push_back(float(image.width()) / float(maxWidth));
        textureSizesData.push_back(float(image.height()) / float(maxHeight));
        zOffset++;
    }

    glGenerateMipmap(GL_TEXTURE_2D_ARRAY);

    glGenTextures(1, &_textureSizes);
    glBindTexture(GL_TEXTURE_1D, _textureSizes);
    // we need to do this even if we're using texelFetch and no sampling is done
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage1D(GL_TEXTURE_1D, 0, GL_RG32F, (GLsizei)textureSizesData.size() / 2, 0, GL_RG, GL_FLOAT, textureSizesData.data());
}

void ModelRenderData::initGeometry(const Model& model)
{
    // vx, vy, vz, nx, ny, nz, s, t, p
    static const int COMPONENTS_PER_VERTEX = 9;

    // we're duplicating a bit to make all this fit the structure of modern graphics APIs
    // the models are so low resolution it should not matter

    vector<float> vertexData;
    vector<uint16_t> indexData;

    for(auto& triangleStrip : model.triangleStrips())
    {
        if(!indexData.empty())
        {
            indexData.push_back(0xFFFF);
        }
        
        for(auto& index : triangleStrip.indices)
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
                vertexData.push_back(1.0f);
                vertexData.push_back(1.0f);
            }
            else
            {
                vertexData.push_back(1.0f - float(vertex.texCoords[index.texCoordIndex].x));
                vertexData.push_back(1.0f - float(vertex.texCoords[index.texCoordIndex].y));
            }

            if(triangleStrip.texIndex == 0xFFFF)
            {
                vertexData.push_back(0.0f);
            }
            else
            {
                vertexData.push_back(float(triangleStrip.texIndex));
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
