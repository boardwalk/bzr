#version 410

layout(location = 0) in vec2 vertexPosition;

out vec3 texCoord;

uniform mat4 rotationMat;

void main()
{
    gl_Position = vec4(vertexPosition, 0.5, 1.0);
    texCoord = (rotationMat * vec4(vertexPosition, 1.0, 1.0)).xyz;
}
