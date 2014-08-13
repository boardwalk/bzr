#version 410 core

in FragmentData
{
    vec3 position;
    vec2 normalTexCoord;
    vec2 terrainTexCoord;
    vec4 terrainInfo1;
    vec4 terrainInfo2;
    vec4 terrainInfo3;
    vec4 terrainInfo4;
    vec4 terrainInfo5;
};

out vec4 fragColor;

#include "graphics/shaders/LandCommon.glsl"

vec3 phong()
{
    vec3 n = texture(normalTex, normalTexCoord).xyz - vec3(0.5);
    n = normalize(normalMatrix * n);
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

vec3 linearize(vec3 color)
{
    return pow(color, vec3(2.2));
}

// Filmic tonemapping operators
// Also applies gamma correction
// http://filmicgames.com/archives/75
vec3 hejl(vec3 color)
{
    vec3 x = max(vec3(0.0), color - vec3(0.004));
    return (x * (6.2 * x + 0.5)) / (x * (6.2 * x + 1.7) + 0.06);
}

float alpha(vec3 texCoord)
{
    vec3 realTexCoord = vec3(texCoord.s, texCoord.t, fract(texCoord.p / 128.0) * 128.0);
    float a = texture(blendTex, realTexCoord).r;
    float z = floor(texCoord.p / 128.0);
    return a + z - 2.0 * a * z;
}

void main()
{
    vec3 tc1 = linearize(texture(terrainTex, vec3(terrainTexCoord.st, terrainInfo1.q)).rgb);
    vec3 tc2 = linearize(texture(terrainTex, vec3(terrainTexCoord.st, terrainInfo2.q)).rgb);
    vec3 tc3 = linearize(texture(terrainTex, vec3(terrainTexCoord.st, terrainInfo3.q)).rgb);
    vec3 tc4 = linearize(texture(terrainTex, vec3(terrainTexCoord.st, terrainInfo4.q)).rgb);
    vec3 tc5 = linearize(texture(terrainTex, vec3(terrainTexCoord.st, terrainInfo5.q)).rgb);

    float ba2 = alpha(terrainInfo2.stp);
    float ba3 = alpha(terrainInfo3.stp);
    float ba4 = alpha(terrainInfo4.stp);
    float ba5 = alpha(terrainInfo5.stp);
 
    vec3 tc = mix(tc2, tc1, ba2);
    tc = mix(tc3, tc, ba3);
    tc = mix(tc4, tc, ba4);
    tc = mix(tc5, tc, ba5);

    fragColor = vec4(hejl(tc) * phong(), 1.0);
}
