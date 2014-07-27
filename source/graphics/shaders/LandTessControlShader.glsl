#version 410

layout (vertices = 3) out;

in FragmentData
{
    vec2 terrainTexCoord;
    vec4 terrainInfo1;
    vec4 terrainInfo2;
    vec4 terrainInfo3;
    vec4 terrainInfo4;
    vec4 terrainInfo5;
} inData[];

out FragmentData
{
    vec2 terrainTexCoord;
    vec4 terrainInfo1;
    vec4 terrainInfo2;
    vec4 terrainInfo3;
    vec4 terrainInfo4;
    vec4 terrainInfo5;
} outData[];

#include "graphics/shaders/LandCommon.glsl"

float GetTessLevel(float distance0, float distance1)
{
    float avgDistance = (distance0 + distance1) / 2.0;
    return max(80.0 / pow(max(avgDistance, 1.0), 0.6), 1.0);
}

void main()
{
    if(gl_InvocationID == 0)
    {
        float vertexDistance0 = distance(cameraPosition, modelMatrix * gl_in[0].gl_Position);
        float vertexDistance1 = distance(cameraPosition, modelMatrix * gl_in[1].gl_Position);
        float vertexDistance2 = distance(cameraPosition, modelMatrix * gl_in[2].gl_Position);

        gl_TessLevelOuter[0] = GetTessLevel(vertexDistance1, vertexDistance2);
        gl_TessLevelOuter[1] = GetTessLevel(vertexDistance2, vertexDistance0);
        gl_TessLevelOuter[2] = GetTessLevel(vertexDistance0, vertexDistance1);
        gl_TessLevelInner[0] = gl_TessLevelOuter[2];
    }

    outData[gl_InvocationID].terrainTexCoord = inData[gl_InvocationID].terrainTexCoord;
    outData[gl_InvocationID].terrainInfo1 = inData[gl_InvocationID].terrainInfo1;
    outData[gl_InvocationID].terrainInfo2 = inData[gl_InvocationID].terrainInfo2;
    outData[gl_InvocationID].terrainInfo3 = inData[gl_InvocationID].terrainInfo3;
    outData[gl_InvocationID].terrainInfo4 = inData[gl_InvocationID].terrainInfo4;
    outData[gl_InvocationID].terrainInfo5 = inData[gl_InvocationID].terrainInfo5;

    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
}
