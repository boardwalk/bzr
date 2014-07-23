#version 410

in FragmentData
{
    vec3 position;
    vec3 normal;
    vec2 terrainTexCoord;
    vec4 terrainInfo1;
    vec4 terrainInfo2;
    vec4 terrainInfo3;
    vec4 terrainInfo4;
    vec4 terrainInfo5;
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
    vec4 tc1 = linearize(texture(terrainTex, vec3(terrainTexCoord.st, terrainInfo1.q)));
    vec4 tc2 = linearize(texture(terrainTex, vec3(terrainTexCoord.st, terrainInfo2.q)));
    vec4 tc3 = linearize(texture(terrainTex, vec3(terrainTexCoord.st, terrainInfo3.q)));
    vec4 tc4 = linearize(texture(terrainTex, vec3(terrainTexCoord.st, terrainInfo4.q)));
    vec4 tc5 = linearize(texture(terrainTex, vec3(terrainTexCoord.st, terrainInfo5.q)));

    float ba2 = texture(blendTex, terrainInfo2.stp).r;
    float ba3 = texture(blendTex, terrainInfo3.stp).r;
    float ba4 = texture(blendTex, terrainInfo4.stp).r;
    float ba5 = texture(blendTex, terrainInfo5.stp).r;

    fragColor = mix(tc2, tc1, ba2);
    fragColor = mix(tc3, fragColor, ba3);
    fragColor = mix(tc4, fragColor, ba4);
    fragColor = mix(tc5, fragColor, ba5);
    fragColor = hejl(fragColor); // * vec4(phong(), 1.0));
}
