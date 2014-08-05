#version 410 core

in vec3 fragTexCoord;

out vec4 fragColor;

uniform sampler2DArray modelTex;

void main()
{
    fragColor = texture(modelTex, fragTexCoord);
}