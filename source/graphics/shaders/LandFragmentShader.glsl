#version 410

in FragmentData
{
    vec3 position;
    vec3 normal;
    vec2 terrainTexCoord;
    vec2 blendTexCoord;
    flat vec4 terrainTexNum;
    flat float roadTexNum;
    flat float blendTexNum;
};

out vec4 fragColor;

uniform sampler2DArray terrainTex;
uniform sampler2DArray blendTex;

uniform vec3 lightPosition;
uniform vec3 lightIntensity;
uniform vec3 Kd;
uniform vec3 Ka;
uniform vec3 Ks;
uniform float shininess;

vec3 phong()
{
    vec3 n = normalize(normal);
    vec3 s = normalize(lightPosition - position);
    vec3 v = normalize(-position);
    vec3 h = normalize(v + s);

    float cosine = max(dot(s, n), 0.0);
    float gapped_cosine = cosine * 0.5 + floor(cosine * 3.0) * 0.25;

    vec3 ambient = Ka;
    vec3 diffuse = Kd * gapped_cosine;
    vec3 specular = Ks * pow(max(dot(h, n), 0.0), shininess);

    return lightIntensity * (ambient + diffuse + specular);
}

vec4 linearize(in vec4 color)
{
	return pow(color, vec4(2.2));
}

// Filmic tonemapping operators
// Also applies gamma correction
// http://filmicgames.com/archives/75
vec4 hejl(in vec4 color)
{
    vec4 x = max(vec4(0.0), color - vec4(0.004));
    return (x * (6.2 * x + 0.5)) / (x * (6.2 * x + 1.7) + 0.06);
}

void main()
{
    // sample terrain lower left, lower right, upper right, upper left
    vec4 tc1 = linearize(texture(terrainTex, vec3(terrainTexCoord.s, terrainTexCoord.t, terrainTexNum.x))) * (1.0 - terrainTexCoord.s) * (1.0 - terrainTexCoord.t);
    vec4 tc2 = linearize(texture(terrainTex, vec3(terrainTexCoord.s, terrainTexCoord.t, terrainTexNum.y))) * terrainTexCoord.s * (1.0 - terrainTexCoord.t);
    vec4 tc3 = linearize(texture(terrainTex, vec3(terrainTexCoord.s, terrainTexCoord.t, terrainTexNum.z))) * terrainTexCoord.s * terrainTexCoord.t;
    vec4 tc4 = linearize(texture(terrainTex, vec3(terrainTexCoord.s, terrainTexCoord.t, terrainTexNum.w))) * (1.0 - terrainTexCoord.s) * terrainTexCoord.t;
    vec4 tc = tc1 + tc2 + tc3 + tc4;

    // sample road
    vec4 rc = linearize(texture(terrainTex, vec3(terrainTexCoord.s, terrainTexCoord.t, roadTexNum)));

    // sample blend
    vec4 a = texture(blendTex, vec3(blendTexCoord.s, blendTexCoord.t, blendTexNum));

    fragColor = hejl(mix(rc, tc, a.r));// * vec4(phong(), 1.0));
}
