#version 410

layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec2 terrainTexCoord;
layout(location = 2) in vec2 blendTexCoord;
layout(location = 3) in vec4 terrainTexNum;
layout(location = 4) in float roadTexNum;
layout(location = 5) in float blendTexNum;

out FragmentData
{
	vec2 terrainTexCoord;
	vec2 blendTexCoord;
	vec4 terrainTexNum;
	float roadTexNum;
	float blendTexNum;
} fragData;

void main()
{
	fragData.terrainTexCoord = terrainTexCoord;
	fragData.blendTexCoord = blendTexCoord;
	fragData.terrainTexNum = terrainTexNum;
	fragData.roadTexNum = roadTexNum;
	fragData.blendTexNum = blendTexNum;

    gl_Position = vec4(vertexPosition * vec3(24.0, 24.0, 2.0), 1.0);
}
