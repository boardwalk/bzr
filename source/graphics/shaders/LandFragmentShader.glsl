#version 410

in FragmentData
{
	vec2 terrainTexCoord;
	vec2 blendTexCoord;
	flat vec4 terrainTexNum;
	flat float roadTexNum;
	flat float blendTexNum;
};

out vec4 fragColor;

uniform sampler2DArray terrainTex;
uniform sampler2DArray blendTex;

void main()
{
	// sample terrain lower left, lower right, upper right, upper left
	vec4 tc1 = texture(terrainTex, vec3(terrainTexCoord.s, terrainTexCoord.t, terrainTexNum.x)) * (1.0 - terrainTexCoord.s) * (1.0 - terrainTexCoord.t);
	vec4 tc2 = texture(terrainTex, vec3(terrainTexCoord.s, terrainTexCoord.t, terrainTexNum.y)) * terrainTexCoord.s * (1.0 - terrainTexCoord.t);
	vec4 tc3 = texture(terrainTex, vec3(terrainTexCoord.s, terrainTexCoord.t, terrainTexNum.z)) * terrainTexCoord.s * terrainTexCoord.t;
	vec4 tc4 = texture(terrainTex, vec3(terrainTexCoord.s, terrainTexCoord.t, terrainTexNum.w)) * (1.0 - terrainTexCoord.s) * terrainTexCoord.t;
	vec4 tc = tc1 + tc2 + tc3 + tc4;

	// sample road
	vec4 rc = texture(terrainTex, vec3(terrainTexCoord.s, terrainTexCoord.t, roadTexNum));

	// sample blend
	vec4 a = texture(blendTex, vec3(blendTexCoord.s, blendTexCoord.t, blendTexNum));

	fragColor = mix(rc, tc, a.r);
}
