#version 410 core

layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 texCoord;

out vec3 fragTexCoord;

uniform mat4 modelViewProjectionMatrix;

void main()
{
    gl_Position = modelViewProjectionMatrix * vec4(vertexPosition, 1.0);
    fragTexCoord = texCoord;
}
