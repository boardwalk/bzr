#version 410

smooth in vec2 fragTexCoord;
out vec4 fragColor;

uniform sampler2D fragTex;

void main()
{
    //fragColor = vec4(1.0, 1.0, 1.0, 1.0);
    fragColor = texture(fragTex, fragTexCoord);
}
