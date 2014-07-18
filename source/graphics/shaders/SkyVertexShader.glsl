#version 410

layout(location = 0) in vec2 vertexPosition;

out vec3 texCoord;

uniform mat4 rotationMat;
uniform mat4 projectionMat;
uniform mat4 viewMat;
uniform mat3 normalMat;

void main()
{
    gl_Position = vec4(vertexPosition, 0.5, 1.0);
    texCoord = (normalMat * vec3(vertexPosition, 1.0));//(rotationMat * vec4(-vertexPosition.x, 1.0, -vertexPosition.y, 1.0)).xyz;
}
