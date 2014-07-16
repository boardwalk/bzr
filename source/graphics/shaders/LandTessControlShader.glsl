#version 410

layout (vertices = 3) out;

in FragmentData
{
    vec2 terrainTexCoord;
    vec2 blendTexCoord;
    vec4 terrainTexNum;
    float roadTexNum;
    float blendTexNum;
} inData[];

out FragmentData
{
    vec2 terrainTexCoord;
    vec2 blendTexCoord;
    vec4 terrainTexNum;
    float roadTexNum;
    float blendTexNum;
} outData[];

void main()
{
    if(gl_InvocationID == 0)
    {
        gl_TessLevelOuter[0] = 5.0;
        gl_TessLevelOuter[1] = 5.0;
        gl_TessLevelOuter[2] = 5.0;
        gl_TessLevelInner[0] = 5.0;
        gl_TessLevelInner[1] = 5.0;
    }

    outData[gl_InvocationID].terrainTexCoord = inData[gl_InvocationID].terrainTexCoord;
    outData[gl_InvocationID].blendTexCoord = inData[gl_InvocationID].blendTexCoord;
    outData[gl_InvocationID].terrainTexNum = inData[gl_InvocationID].terrainTexNum;
    outData[gl_InvocationID].roadTexNum = inData[gl_InvocationID].roadTexNum;
    outData[gl_InvocationID].blendTexNum = inData[gl_InvocationID].blendTexNum;

    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
}
