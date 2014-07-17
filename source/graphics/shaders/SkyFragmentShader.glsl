#version 410

in vec3 texCoord;

out vec4 fragColor;

uniform samplerCube cubeTex;

void main()
{
    fragColor = vec4(1.0, 1.0, 1.0, 1.0); //texture(cubeTex, texCoord);
}

