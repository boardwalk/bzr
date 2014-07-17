#include "graphics/LandblockRenderData.h"
#include "graphics/Program.h"
#include "math/Vec2.h"
#include "Landblock.h"

LandblockRenderData::LandblockRenderData(const Landblock& landblock)
{
    initVAO(landblock);
    initHeightTexture(landblock);
}

LandblockRenderData::~LandblockRenderData()
{
    glDeleteVertexArrays(1, &_vertexArray);
    glDeleteBuffers(1, &_vertexBuffer);
    glDeleteTextures(1, &_heightTexture);
}

void LandblockRenderData::bind(Program& program)
{
    glBindVertexArray(_vertexArray);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, _heightTexture);

    auto heightBaseLoc = program.getUniform("heightBase");
    glUniform1f(heightBaseLoc, _heightBase);

    auto heightScaleLoc = program.getUniform("heightScale");
    glUniform1f(heightScaleLoc, _heightScale);
}

GLsizei LandblockRenderData::vertexCount() const
{
	return _vertexCount;
}

static Vec2 rotateTexCoord(Vec2 tc, double deg)
{
    auto cosine = cos(deg / 180.0 * PI);
    auto sine = sin(deg / 180.0 * PI);
    return Vec2(
        (tc.x - 0.5) * cosine - (tc.y - 0.5) * sine + 0.5,
        (tc.x - 0.5) * sine + (tc.y - 0.5) * cosine + 0.5);
}

void LandblockRenderData::initVAO(const Landblock& landblock)
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

            auto blendAngle = 0.0;

            // road texture number
            auto rp = 0x20;

            // blend texture number
            auto bp = 0;

            auto scale = 3.0;

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
                    blendAngle = 270.0;
                    break;
                case 0x0011: // north road
                    bp = 2;
                    blendAngle = 90.0;
                    break;
                case 0x1001: // west road
                    bp = 2;
                    blendAngle = 0.0;
                    break;
                case 0x0110: // east road
                    blendAngle = 180.0;
                    bp = 2;
                    break;
                case 0x1000: // southwest corner
                    bp = 3;
                    blendAngle = 0.0;
                    break;
                case 0x0100: // southeast corner
                    bp = 3;
                    blendAngle = 270.0;
                    break;
                case 0x0010: // northeast corner
                    bp = 4;
                    blendAngle = 180.0;
                    break;
                case 0x0001: // northwest corner
                    bp = 4;
                    blendAngle = 90.0;
                    break;
                case 0x1010: // southwest/northeast diagonal
                    bp = 10;
                    scale = 1.0;
                    break;
                case 0x0101: // southeast/northwest diagonal
                    bp = 10;
                    blendAngle = 90.0;
                    scale = 1.0;
                    break;
                case 0x1110:
                    //printf("lower right road\n");
                    bp = 3;
                    break;
                case 0x0111:
                    //printf("upper right road\n");
                    bp = 3;
                    break;
                case 0x1011:
                    //printf("upper left road\n");
                    bp = 3;
                    break;
                case 0x1101:
                    //printf("upper right road\n");
                    bp = 3;
                    break;
                default:
                    assert(false);
                    break;
            }

// See LandVertexShader.glsl too see what these are
#define V(dx, dy) \
    vertexData.push_back((x + (dx)) * 24); \
    vertexData.push_back((y + (dy)) * 24); \
    vertexData.push_back(dx); \
    vertexData.push_back(dy); \
    { \
        auto tcr = rotateTexCoord(Vec2(dx, dy), blendAngle); \
        vertexData.push_back(scale * tcr.x + 0.5); /* ghetto rounding */ \
        vertexData.push_back(scale * tcr.y + 0.5); /* ghetto rounding */ \
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
}

void LandblockRenderData::initHeightTexture(const Landblock& landblock)
{
    glGenTextures(1, &_heightTexture);
    glBindTexture(GL_TEXTURE_2D, _heightTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R16, Landblock::HEIGHT_MAP_SIZE, Landblock::HEIGHT_MAP_SIZE, 0, GL_RED, GL_UNSIGNED_SHORT, landblock.getHeightMap());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); // default is GL_NEAREST_MIPMAP_LINEAR
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    _heightBase = landblock.getHeightMapBase();
    _heightScale = landblock.getHeightMapScale();
}
