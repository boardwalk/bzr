#version 410

layout(triangles, fractional_even_spacing, ccw) in;

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
    vec3 position;
    vec3 normal;
    vec2 terrainTexCoord;
    vec2 blendTexCoord;
    flat vec4 terrainTexNum;
    flat float roadTexNum;
    flat float blendTexNum;
} outData;

uniform mat3 normalMatrix;
uniform mat4 modelViewMatrix;
uniform mat4 modelViewProjectionMatrix;

uniform sampler2D heightTex;
uniform float heightBase;
uniform float heightScale;

void main()
{
    vec4 modelPos = gl_in[0].gl_Position * gl_TessCoord.x + 
                    gl_in[1].gl_Position * gl_TessCoord.y +
                    gl_in[2].gl_Position * gl_TessCoord.z;

    float height = heightBase + texture(heightTex, modelPos.xy / 192.0).r * heightScale;

    gl_Position = modelViewProjectionMatrix * vec4(modelPos.xy, height, 1.0);

    outData.position = vec3(modelViewMatrix * vec4(modelPos.xy, height, 1.0));

    vec3 a = gl_in[1].gl_Position.xyz - gl_in[0].gl_Position.xyz;
    vec3 b = gl_in[2].gl_Position.xyz - gl_in[0].gl_Position.xyz;
    outData.normal = normalMatrix * cross(a, b);

    outData.terrainTexCoord = inData[0].terrainTexCoord * gl_TessCoord.x +
                              inData[1].terrainTexCoord * gl_TessCoord.y +
                              inData[2].terrainTexCoord * gl_TessCoord.z;

    outData.blendTexCoord = inData[0].blendTexCoord * gl_TessCoord.x +
                            inData[1].blendTexCoord * gl_TessCoord.y +
                            inData[2].blendTexCoord * gl_TessCoord.z;

    outData.terrainTexNum = inData[0].terrainTexNum;

    outData.roadTexNum = inData[0].roadTexNum;

    outData.blendTexNum = inData[0].blendTexNum;

    
}
