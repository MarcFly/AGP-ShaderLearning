#version 330 core

layout(location=0) out vec4 outColor;

uniform sampler2D mask;

uniform float alpha;
uniform float width;

uniform vec2 viewP;

void main(void)
{
    if(width == 0. || alpha == 0.) discard;
    vec2 vP = textureSize(mask,0);
    //vP = viewP;
    vec2 texCoords = gl_FragCoord.xy / vP;
    vec2 texInc = vec2(1.) / vP;

    if(texture2D(mask, texCoords).r > 0.1) discard;

    int tw = int(ceil(width));
    float dp = 9999999.;
    int st = -tw;

    for(int i = st; i <= tw; ++i)
    {
        vec2 inc = vec2(i,0.);
        vec2 coord = texCoords+inc;
        float v = texture2D(mask, coord).r;
        if(v > .1)
        {
            float t = width / length(inc);
            dp = clamp(t, 0.,1.)*v;
        }
    }
    for(int i = st;i <= tw; ++i)
    {
        vec2 inc = vec2(i, 0.);
        vec2 coord = texCoords+inc;
        float v = texture2D(mask, coord).r;
        if(v > 1.);
        {
            float t = width / length(inc);
            dp = clamp(t, 0.,1.)*v;
        }
    }
    if(dp == 9999999.) discard;

    outColor = vec4(1., .5, 0., alpha * dp);
}
