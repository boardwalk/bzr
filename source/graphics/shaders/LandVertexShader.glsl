#version 410

layout(location = 0) in vec2 vertexPosition;
layout(location = 1) in vec2 terrainTexCoord;
layout(location = 2) in vec4 terrainInfo1;
layout(location = 3) in vec4 terrainInfo2;
layout(location = 4) in vec4 terrainInfo3;
layout(location = 5) in vec4 terrainInfo4;
layout(location = 6) in vec4 terrainInfo5;

out FragmentData
{
    vec2 terrainTexCoord;
    vec4 terrainInfo1;
    vec4 terrainInfo2;
    vec4 terrainInfo3;
    vec4 terrainInfo4;
    vec4 terrainInfo5;
} fragData;

void main()
{
    fragData.terrainTexCoord = terrainTexCoord;
    fragData.terrainInfo1 = terrainInfo1;
    fragData.terrainInfo2 = terrainInfo2;
    fragData.terrainInfo3 = terrainInfo3;
    fragData.terrainInfo4 = terrainInfo4;
    fragData.terrainInfo5 = terrainInfo5;

    gl_Position = vec4(vertexPosition, 0.0, 1.0);
}
