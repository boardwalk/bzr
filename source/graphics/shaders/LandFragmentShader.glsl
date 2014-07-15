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
	// sample terrain lower left, lower right, upper right, upper left
	vec4 tc1 = texture(terrainTex, vec3(fragTerrainTexCoord.s, fragTerrainTexCoord.t, fragTerrainTexNum.x)) * (1.0 - fragTerrainTexCoord.s) * (1.0 - fragTerrainTexCoord.t);
	vec4 tc2 = texture(terrainTex, vec3(fragTerrainTexCoord.s, fragTerrainTexCoord.t, fragTerrainTexNum.y)) * fragTerrainTexCoord.s * (1.0 - fragTerrainTexCoord.t);
	vec4 tc3 = texture(terrainTex, vec3(fragTerrainTexCoord.s, fragTerrainTexCoord.t, fragTerrainTexNum.z)) * fragTerrainTexCoord.s * fragTerrainTexCoord.t;
	vec4 tc4 = texture(terrainTex, vec3(fragTerrainTexCoord.s, fragTerrainTexCoord.t, fragTerrainTexNum.w)) * (1.0 - fragTerrainTexCoord.s) * fragTerrainTexCoord.t;
	vec4 tc = tc1 + tc2 + tc3 + tc4;

	// sample road
	vec4 rc = texture(terrainTex, vec3(fragTerrainTexCoord.s, fragTerrainTexCoord.t, fragRoadTexNum));

	// sample blend
	vec4 a = texture(blendTex, vec3(fragBlendTexCoord.s, fragBlendTexCoord.t, fragBlendTexNum));

	fragColor = mix(rc, tc, a.r);
}
