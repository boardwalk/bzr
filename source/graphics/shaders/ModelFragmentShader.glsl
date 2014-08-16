#version 410 core

in vec3 fragTexCoord;

out vec4 fragColor;

uniform sampler2DArray modelTex;
uniform sampler1D modelTexSizes;

void main()
{
    vec4 texSize = texelFetch(modelTexSizes, int(fragTexCoord.p), 0);
    vec3 scaledFragTexCoord = vec3((1.0 - mod(fragTexCoord.st, 1.0)) * texSize.st, fragTexCoord.p);
    fragColor = texture(modelTex, scaledFragTexCoord);
}
