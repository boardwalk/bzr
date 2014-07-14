#version 410

layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec2 terrainTexCoord;
layout(location = 2) in vec2 blendTexCoord;
layout(location = 3) in vec4 terrainTexNum;
layout(location = 4) in float roadTexNum;
layout(location = 5) in float blendTexNum;

out vec2 fragTerrainTexCoord;
out vec2 fragBlendTexCoord;
out vec4 fragTerrainTexNum;
out float fragRoadTexNum;
out float fragBlendTexNum;

uniform mat4 modelViewProjectionMatrix;

void main()
{
	fragTerrainTexCoord = terrainTexCoord;
	fragBlendTexCoord = blendTexCoord;
	fragTerrainTexNum = terrainTexNum;
	fragRoadTexNum = roadTexNum;
	fragBlendTexNum = blendTexNum;

    gl_Position = modelViewProjectionMatrix * vec4(vertexPosition * vec3(24.0, 24.0, 2.0), 1.0);
}
