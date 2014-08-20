#version 410 core

in vec3 fragTexCoord;

out vec4 fragColor;

uniform sampler2D atlas;
uniform sampler1D atlasToc;

void main()
{
    // stpq -- st, normalized lower left; pq, normalized width and height
    vec4 tileExtents = texelFetch(atlasToc, int(fragTexCoord.p + 0.5), 0);
    vec2 scaledFragTexCoord = tileExtents.st + (vec2(1.0) - fract(fragTexCoord.st)) * tileExtents.pq;
    fragColor = texture(atlas, scaledFragTexCoord);
}
