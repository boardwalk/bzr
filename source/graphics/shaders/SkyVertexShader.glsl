#version 410

layout(location = 0) in vec3 vertexPosition;

uniform mat4 rotationMat;

out vec3 texCoord;

void main()
{
    gl_Position = vec4(vertexPosition, 1.0);
    //texCoord = (rotationMat * vec4(vertexPosition, 1.0)).xyz;
    //texCoord = vertexPosition;

    mat4 foo = mat4(1.0);
    texCoord = (foo * vec4(vertexPosition, 1.0)).xyz;
}

