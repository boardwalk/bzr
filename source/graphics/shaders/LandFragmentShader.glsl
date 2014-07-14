#version 410

in vec2 fragTerrainTexCoord;
in vec2 fragBlendTexCoord;
in vec4 fragTerrainTexNum; // flat?
in float fragRoadTexNum; // flat?
in float fragBlendTexNum; // flat?

out vec4 fragColor;

uniform sampler2DArray terrainTex;
uniform sampler2DArray blendTex;

void main()
{
	// lower left
	vec4 c1 = texture(terrainTex, vec3(fragTerrainTexCoord.s, fragTerrainTexCoord.t, fragTerrainTexNum.x)) * (1.0 - fragTerrainTexCoord.s) * (1.0 - fragTerrainTexCoord.t);
	// lower right
	vec4 c2 = texture(terrainTex, vec3(fragTerrainTexCoord.s, fragTerrainTexCoord.t, fragTerrainTexNum.y)) * fragTerrainTexCoord.s * (1.0 - fragTerrainTexCoord.t);
	// upper right
	vec4 c3 = texture(terrainTex, vec3(fragTerrainTexCoord.s, fragTerrainTexCoord.t, fragTerrainTexNum.z)) * fragTerrainTexCoord.s * fragTerrainTexCoord.t;
	// upper left
	vec4 c4 = texture(terrainTex, vec3(fragTerrainTexCoord.s, fragTerrainTexCoord.t, fragTerrainTexNum.w)) * (1.0 - fragTerrainTexCoord.s) * fragTerrainTexCoord.t;

	fragColor = c1 + c2 + c3 + c4;
}
