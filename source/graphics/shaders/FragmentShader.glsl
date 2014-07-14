#version 410

in vec3 position;
in vec3 normal;
in vec3 fragTexCoord;

uniform vec4 lightPosition;
uniform vec3 lightIntensity;
uniform vec3 Kd;
uniform vec3 Ka;
uniform vec3 Ks;
uniform float shininess;
uniform sampler2DArray fragTex;

out vec4 fragColor;

vec3 phong()
{
    vec3 n = normalize(normal);
    vec3 s = normalize(vec3(lightPosition) - position);
    vec3 v = normalize(-position);
    vec3 h = normalize(v + s);

    float cosine = max(dot(s, n), 0.0);
    float gapped_cosine = cosine * 0.5 + floor(cosine * 3.0) * 0.25;

    vec3 ambient = Ka;
    vec3 diffuse = Kd * gapped_cosine;
    vec3 specular = Ks * pow(max(dot(h, n), 0.0), shininess);

    return lightIntensity * (ambient + diffuse + specular);
}

// Filmic tonemapping operators
// Also applies gamma correction
// http://filmicgames.com/archives/75
vec4 hejl(in vec4 color) {
    vec4 x = max(vec4(0.0), color - vec4(0.004));
    return (x * (6.2 * x + 0.5)) / (x * (6.2 * x + 1.7) + 0.06);
}

void main()
{
    // convert non-linear to linear
    fragColor = pow(texture(fragTex, fragTexCoord), vec4(2.2));

    // apply lighting
    fragColor = vec4(phong(), 1.0) * fragColor;

    // convert back to non-linear
    fragColor = hejl(fragColor);
}
