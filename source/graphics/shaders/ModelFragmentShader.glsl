#version 410 core

in vec3 fragTexCoord;

out vec4 fragColor;

uniform sampler2D atlas;
uniform sampler1D atlasToc;

void main()
{
    // stpq -- st, normalized lower left; pq, normalized width and height
    vec4 tileExtents = texelFetch(atlasToc, int(fragTexCoord.p), 0);
    vec2 scaledFragTexCoord = tileExtents.st + mod(fragTexCoord.st, 1.0) * tileExtents.pq;
    fragColor = texture(atlas, scaledFragTexCoord);
}
