#version 330 core

layout(location=0) out vec4 outColor;

uniform sampler2D mask;

uniform float alpha;
uniform float width;

void main(void)
{
    if(width == 0. || alpha == 0.) discard;
    vec2 vP = textureSize(mask,0);
    vec2 texCoords = gl_FragCoord.xy / vP;
    vec2 texInc = vec2(1.) / vP;

    bool found = false;
    float testwidth = ceil(width);
    float dist_perc = 1.;
    for(float i = -width; i < testwidth; ++i)
    {
        for(float j = -width; j < testwidth; ++j)
        {
            vec2 inc = vec2(i,j);
            found = (texture2D(mask, texCoords + inc).r == 1.);
            if(found) dist_perc = clamp(width / length(inc), 0., 1.);
        }
    }

    if(!found) discard;

    outColor = vec4(1., .5, 0., alpha * dist_perc);
}
