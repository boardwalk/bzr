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
	vec2 terrainTexCoord;
	vec2 blendTexCoord;
	flat vec4 terrainTexNum;
	flat float roadTexNum;
	flat float blendTexNum;
} outData;

void main()
{
	outData.terrainTexCoord = inData[0].terrainTexCoord * gl_TessCoord.x +
							  inData[1].terrainTexCoord * gl_TessCoord.y +
							  inData[2].terrainTexCoord * gl_TessCoord.z;

	outData.blendTexCoord = inData[0].blendTexCoord * gl_TessCoord.x +
						    inData[1].blendTexCoord * gl_TessCoord.y +
							inData[2].blendTexCoord * gl_TessCoord.z;

	outData.terrainTexNum = inData[0].terrainTexNum;

    outData.roadTexNum = inData[0].roadTexNum;

    outData.blendTexNum = inData[0].blendTexNum;

	gl_Position = gl_in[0].gl_Position * gl_TessCoord.x + 
				  gl_in[1].gl_Position * gl_TessCoord.y +
				  gl_in[2].gl_Position * gl_TessCoord.z;
}
