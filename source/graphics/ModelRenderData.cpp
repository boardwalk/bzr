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

void ModelRenderData::initTexture(const Model& model)
{
    // Choose common texture format
    // Size array texture by largest texture used
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

        if(image.format() != Image::BGRA32)
        {
            continue;
        }

        maxWidth = max(maxWidth, GLsizei(image.width()));
        maxHeight = max(maxHeight, GLsizei(image.height()));
    }

    glGenTextures(1, &_texture);
    glBindTexture(GL_TEXTURE_2D_ARRAY, _texture);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA8, maxWidth, maxHeight, (GLsizei)model.textures().size(), 0, GL_BGRA, GL_UNSIGNED_BYTE, nullptr);   

    GLint zoffset = 0;

    for(auto& resource : model.textures())
    {
        Image image;

        auto innerResource = getTexture(resource);

        if(innerResource)
        {
            image = innerResource->cast<Texture>().image();
            image.scale(maxWidth, maxHeight);
        }

        if(image.format() == Image::BGRA32)
        {
            glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, zoffset, image.width(), image.height(), 1, GL_BGRA, GL_UNSIGNED_BYTE, image.data());
        }
        else if(image.format() == Image::BGR24)
        {
            glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, zoffset, image.width(), image.height(), 1, GL_BGR, GL_UNSIGNED_BYTE, image.data());
        }
        else
        {
            image.init(Image::BGR24, maxWidth, maxHeight, nullptr);
            image.fill(0xFF);

            glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, zoffset, image.width(), image.height(), 1, GL_BGR, GL_UNSIGNED_BYTE, image.data());
        }

        zoffset++;
    }

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

    // TODO The origin of DirectX's texcoord is the top left, OpenGL is bottom right
    // We may need to correct for this

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
