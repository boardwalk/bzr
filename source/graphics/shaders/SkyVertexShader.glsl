#version 410

layout(location = 0) in vec3 vertexPosition;

uniform mat4 modelViewProjectionMatrix;

out vec3 texCoord;

void main()
{
    gl_Position = modelViewProjectionMatrix * vec4(vertexPosition, 1.0) * 20.0;
    texCoord = vertexPosition;
}

