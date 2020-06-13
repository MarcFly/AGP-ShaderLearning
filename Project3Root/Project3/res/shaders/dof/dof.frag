#version 330 core

layout(location=0) out float outColor;

uniform float fDist;
uniform float fDepth;
uniform float fFalloff;

uniform vec2 z;

uniform sampler2D depth;

in vec2 texCoord;

float viz(float v, float f, float n)
{
    return abs((2 * f * n) / ((v * 2.0 - 1.0) *(f-n)-(f+n))) / 50.;
}

void main()
{
    float f = z.y;
    float n = z.x;

    float texel = texture2D(depth, texCoord).r;

    // linear test
    float ndc = texel * 2. - 1.;
    float rD = 2.0 * n * f / (f + n - ndc * (f - n));

    // Value if between fDist +- fDepth = 0
    // Value over fDist+- fDepth starts falling off and reaches max at fDist+(fFalloff * fDepth/2)

    float val = clamp(abs(fDist - rD) / fDepth, 0.,1.);
    val = (abs(fDist - rD) - fDepth);
    val = val / fFalloff;

    outColor = val;
}
