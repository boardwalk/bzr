#version 410

layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec3 vertexNormal;
layout(location = 2) in vec2 texCoord;

uniform mat3 normalMatrix;
uniform mat4 modelViewMatrix;
uniform mat4 modelViewProjectionMatrix;

out vec3 position;
out vec3 normal;
out vec2 fragTexCoord;

void main()
{
    position = vec3(modelViewMatrix * vec4(vertexPosition, 1.0));
    normal = normalize(normalMatrix * vertexNormal);
    fragTexCoord = texCoord;
    gl_Position = modelViewProjectionMatrix * vec4(vertexPosition, 1.0);
}
