#version 330 core

out float outColor;

uniform sampler2D colorMap;
uniform sampler2D Mask;

uniform vec2 dir;
uniform float ratio;

in vec2 texCoord;

// Kernel Calculator?
// http://dev.theomader.com/gaussian-kernel-calculator/

void main()
{

    float weights[11];
    weights[0] = 0.035822;
    weights[1] = 0.05879;
    weights[2] = 0.086425;
    weights[3] = 0.113806;
    weights[4] = 0.13424;
    weights[5] = 0.141836;
    weights[10] = 0.035822;
    weights[9] = 0.05879;
    weights[8] = 0.086425;
    weights[7] = 0.113806;
    weights[6] = 0.13424;

    vec2 viewP = textureSize(colorMap, 0).xy;

    float baseCol = texture2D(colorMap, texCoord).r;
    float maskval = clamp(texture2D(Mask, texCoord).r, 0., 1.);

    float f = 10000.0;
    float n = 0.01;

    vec2 dir_corrected = normalize(dir) / viewP;

    float sumweights = 0.;
    float blurCol = 0.;
    vec2 uv = texCoord - dir_corrected * 5.;
    for(int j = 0; j < 11; ++j)
    {
        // Maskn check that pixel if it is affected
        // To make sure blur does not bleed too much into
        // other areas, it leverages that effect around those
        // unmasked areas

        float maskn = clamp(texture2D(Mask, uv).r, 0.,1.);

        blurCol += texture2D(colorMap, uv).r * weights[j]*maskn;
        uv += dir_corrected;
        sumweights += (weights[j] * maskn);
    }
    blurCol /= sumweights;

    float finalCol = mix(baseCol, blurCol, ratio*maskval);
    outColor = finalCol;

}
