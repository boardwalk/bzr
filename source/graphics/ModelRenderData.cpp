#include "graphics/ModelRenderData.h"
#include "Model.h"
#include "Texture.h"
#include "TextureLookup5.h"
#include "TextureLookup8.h"
#include <vector>

struct TextureGLFormat
{
    GLenum internalFormat;
    GLenum format;
    GLenum type;
    bool compressed;
};

static void getGLFormat(Texture::Type type, TextureGLFormat& format)
{
    switch(type)
    {
        case Texture::BGR24:
            format.internalFormat = GL_RGB8;
            format.format = GL_BGR;
            format.type = GL_UNSIGNED_BYTE;
            format.compressed = false;
            break;

        case Texture::BGRA32:
            format.internalFormat = GL_RGBA8;
            format.format = GL_BGRA;
            format.type = GL_UNSIGNED_BYTE;
            format.compressed = false;
            break;

        case Texture::Paletted16:
            // TODO We should really be applying the palette!
            format.internalFormat = GL_R16;
            format.format = GL_R;
            format.type = GL_UNSIGNED_SHORT;
            format.compressed = false;
            break;

        case Texture::RGB24:
            format.internalFormat = GL_RGB8;
            format.format = GL_RGB;
            format.type = GL_UNSIGNED_BYTE;
            format.compressed = false;
            break;

        case Texture::Alpha8:
            format.internalFormat = GL_R8;
            format.format = GL_R;
            format.type = GL_UNSIGNED_BYTE;
            format.compressed = false;
            break;

        case Texture::DXT1:
            format.internalFormat = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;//GL_COMPRESSED_RGBA
            format.format = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
            format.type = GL_UNSIGNED_BYTE;
            format.compressed = true;
            break;

        case Texture::DXT5:
            format.internalFormat = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;//GL_COMPRESSED_RGBA;
            format.format = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
            format.type = GL_UNSIGNED_BYTE;
            format.compressed = true;
            break;
    }
}

ModelRenderData::ModelRenderData(const Model& model)
{
    vector<Vec2> texCoordScales;
    initTexture(model, texCoordScales);
    initGeometry(model, texCoordScales);
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
            for(auto& subtexture : resource->cast<TextureLookup5>().textures())
            {
                if(subtexture)
                {
                    return getTexture(subtexture);
                }
            }

            return ResourcePtr();

        case Resource::TextureLookup8:
            return getTexture(resource->cast<TextureLookup8>().texture());

        default:
            return ResourcePtr();
    }
}

static void checkGLError()
{
    auto err = glGetError();

    if(err != GL_NO_ERROR)
    {
        printf("Got GL error: %x\n", err);
    }
}

void ModelRenderData::initTexture(const Model& model, vector<Vec2>& texCoordScales)
{
    printf("initTexture()\n");

    // Choose common texture format
    // Size array texture by largest texture used
    auto firstTexture = true;
    Texture::Type type = Texture::Invalid;
    GLsizei maxWidth = 0;
    GLsizei maxHeight = 0;

    for(auto& resource : model.textures())
    {
        auto innerResource = getTexture(resource);

        if(!innerResource)
        {
            continue;
        }

        auto& texture = innerResource->cast<Texture>();

        if(firstTexture)
        {
            firstTexture = false;
            type = texture.type();
        }
        else if(texture.type() != type)
        {
            throw runtime_error("Multiple texture formats used in model");
        }

        maxWidth = max(maxWidth, GLsizei(texture.width()));
        maxHeight = max(maxHeight, GLsizei(texture.height()));
    }

    TextureGLFormat format;
    getGLFormat(type, format);

    glGenTextures(1, &_texture);
    glBindTexture(GL_TEXTURE_2D_ARRAY, _texture);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);

    if(format.compressed)
    {
        glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, format.internalFormat, maxWidth, maxHeight, (GLsizei)model.textures().size(), 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);   
    }
    else
    {
        glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, format.internalFormat, maxWidth, maxHeight, (GLsizei)model.textures().size(), 0, format.format, format.type, nullptr);
    }

    checkGLError();

    GLint zoffset = 0;

    texCoordScales.resize(model.textures().size());

    for(auto& resource : model.textures())
    {
        auto innerResource = getTexture(resource);

        if(!innerResource)
        {
            printf("skip!\n");
            zoffset++;
            continue;
        }

        auto& texture = innerResource->cast<Texture>();

        if(format.compressed)
        {
            printf("compressed! %d %d %d\n", zoffset, texture.width(), texture.height());
            glCompressedTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, zoffset, texture.width(), texture.height(), 1, format.format, GLsizei(texture.pixels().size()), texture.pixels().data());
            checkGLError();
        }
        else
        {
            printf("uncompressed! %d %d %d\n", zoffset, texture.width(), texture.height());
            glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, zoffset, texture.width(), texture.height(), 1, format.format, format.type, texture.pixels().data());
            checkGLError();
        }

        texCoordScales[zoffset].x = double(texture.width()) / double(maxWidth);
        texCoordScales[zoffset].y = double(texture.height()) / double(maxHeight);

        zoffset++;
    }

    glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
    checkGLError();
}

void ModelRenderData::initGeometry(const Model& model, const vector<Vec2>& texCoordScales)
{
    // vx, vy, vz, nx, ny, nz, s, t, p
    static const int COMPONENTS_PER_VERTEX = 9;

    // we're deduplicating a bit to make all this fit the structure of modern graphics APIs
    // the models are so low resolution it should not matter

    vector<float> vertexData;
    vector<uint16_t> indexData;

    for(auto& primitive : model.primitives())
    {
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

            if(primitive.texIndex == 0xFF)
            {
                vertexData.push_back(0.0f);
            }
            else
            {
                vertexData.push_back(float(primitive.texIndex));
            }

            if(vertex.texCoords.empty())
            {
                vertexData.push_back(0.0);
                vertexData.push_back(0.0);
            }
            else
            {
                vertexData.push_back(float(vertex.texCoords[index.texCoordIndex].x * texCoordScales[index.texCoordIndex].x));
                vertexData.push_back(float(vertex.texCoords[index.texCoordIndex].y * texCoordScales[index.texCoordIndex].y));
            }
        }

        indexData.push_back(0xFFFF);
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
