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

void main()
{
    if(gl_InvocationID == 0)
    {
        gl_TessLevelOuter[0] = 10.0;
        gl_TessLevelOuter[1] = 10.0;
        gl_TessLevelOuter[2] = 10.0;
        gl_TessLevelInner[0] = 10.0;
        gl_TessLevelInner[1] = 10.0;
    }

    outData[gl_InvocationID].terrainTexCoord = inData[gl_InvocationID].terrainTexCoord;
    outData[gl_InvocationID].terrainInfo1 = inData[gl_InvocationID].terrainInfo1;
    outData[gl_InvocationID].terrainInfo2 = inData[gl_InvocationID].terrainInfo2;
    outData[gl_InvocationID].terrainInfo3 = inData[gl_InvocationID].terrainInfo3;
    outData[gl_InvocationID].terrainInfo4 = inData[gl_InvocationID].terrainInfo4;
    outData[gl_InvocationID].terrainInfo5 = inData[gl_InvocationID].terrainInfo5;

    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
}
