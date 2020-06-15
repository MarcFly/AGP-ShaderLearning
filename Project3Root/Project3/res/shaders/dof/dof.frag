#version 330 core

layout(location=0) out float outColor;

uniform float fDist;
uniform float fDepth;
uniform float fFalloff;

uniform vec2 z;

uniform sampler2D depth;

uniform bool bleed;

in vec2 texCoord;

float calcBlur(float texel, float f, float n)
{
    // linear test
    float ndc = texel * 2. - 1.;
    float rD = 2.0 * n * f / (f + n - ndc * (f - n));

    // Value if between fDist +- fDepth = 0
    // Value over fDist+- fDepth starts falling off and reaches max at fDist+(fFalloff * fDepth/2)

    float val = clamp(abs(fDist - rD) / fDepth, 0.,1.);
    val = (abs(fDist - rD) - fDepth);
    return val / fFalloff;
}

void main()
{
    float f = z.y;
    float n = z.x;

    float d = texture2D(depth, texCoord).r;

    float val = calcBlur(d, f, n);
    //val = 0;

    // Try working an 11x11 kernel to test bleeding
    if(bleed)
    {
        float sumweights = 0.;
        float blurCol = 0.;
        vec2 uv = vec2(1. / textureSize(depth, 0));
        for(int i = -5; i < 6; ++i)
        {
            for(int j = -5;  j < 6; ++j)
            {
                float d2 = texture2D(depth, texCoord + uv*vec2(i,j)).r;
                //d2 = abs((2 * f * n) / ((d2 * 2.0 - 1.0) *(f-n)-(f+n)))/50.;

                float fb = (d - d2 < .001 ? 0. : 1.);

                blurCol += calcBlur(d2,f,n)*fb;
                sumweights += fb;
            }
        }
        blurCol /= sumweights;

        //blurCol = sumweights / (11*11);

        if( val <= 0.1)
        val += blurCol;
    }

    outColor = clamp(val, 0., 1.);
}
