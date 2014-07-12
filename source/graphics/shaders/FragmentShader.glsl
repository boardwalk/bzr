#version 410

in vec3 position;
in vec3 normal;

uniform vec4 lightPosition;
uniform vec3 lightIntensity;
uniform vec3 Kd;
uniform vec3 Ka;
uniform vec3 Ks;
uniform float shininess;

out vec4 fragColor;

vec3 phong() {
    vec3 n = normalize(normal);
    vec3 s = normalize(vec3(lightPosition) - position);
    vec3 v = normalize(-position);
    vec3 h = normalize(v + s);
    return
        lightIntensity * (Ka +
             Kd * max(dot(s, n), 0.0) +
             Ks * pow(max(dot(h, n), 0.0), shininess));
}

void main()
{
    fragColor = vec4(phong(), 1.0);
}
