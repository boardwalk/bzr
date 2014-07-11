#version 410

layout(location = 0) in vec3 vert;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

void main()
{
    gl_Position = projection * view * model * vec4(vert.x, vert.y, vert.z, 1.0);
}
