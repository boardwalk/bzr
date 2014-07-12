#version 410

layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec3 vertexNormal;

uniform mat3 normalMatrix;
uniform mat4 modelViewMatrix;
uniform mat4 modelViewProjectionMatrix;

out vec3 position;
out vec3 normal;

void main()
{
    position = vec3(modelViewMatrix * vec4(vertexPosition, 1.0));
    normal = normalize(normalMatrix * vertexNormal);
    gl_Position = modelViewProjectionMatrix * vec4(vertexPosition, 1.0);
}
