#version 410 core

in vec3 fragTexCoord;

out vec4 fragColor;

uniform sampler2D atlas;
uniform sampler1D atlasToc;

void main()
{
    // stpq -- st, normalized lower left; pq, normalized width and height
    vec4 tileExtents = texelFetch(atlasToc, int(fragTexCoord.p + 0.5), 0);

    vec2 tileTexCoord = vec2(1.0) - fract(fragTexCoord.st);
    vec2 atlasTexCoord = tileExtents.st + tileTexCoord * tileExtents.pq;

    vec2 dx = dFdx(fragTexCoord.st * tileExtents.pq);
    vec2 dy = dFdy(fragTexCoord.st * tileExtents.pq);
    //vec2 dx = dFdx(atlasTexCoord);
    //vec2 dy = dFdy(atlasTexCoord);

    fragColor = textureGrad(atlas, atlasTexCoord, dx, dy);
}
