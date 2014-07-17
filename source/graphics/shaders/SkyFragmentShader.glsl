#version 410

in vec3 texCoord;

out vec4 fragColor;

uniform samplerCube cubeTex;

void main()
{
    fragColor = texture(cubeTex, texCoord);
}
