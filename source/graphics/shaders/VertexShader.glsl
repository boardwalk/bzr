#version 410

layout(location = 0) in vec3 vert;
layout(location = 1) in vec2 texCoord;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

varying vec2 fragTexCoord;

void main()
{
    gl_Position = projection * view * model * vec4(vert.x, vert.y, vert.z, 1.0);
    fragTexCoord = texCoord;
}

