#version 330 core

out vec4 outColor;

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

    vec3 baseCol = texture2D(colorMap, texCoord).rgb;
    float maskval = clamp(texture2D(Mask, texCoord).r, 0., 1.);

    vec2 dir_corrected = normalize(dir) / viewP;

    float sumweights = 0.;
    vec3 blurCol = vec3(0.);
    vec2 uv = texCoord - dir_corrected * 5.;
    for(int j = 0; j < 11; ++j)
    {
        // Maskn check that pixel if it is affected
        // To make sure blur does not bleed too much into
        // other areas, it leverages that effect around those
        // unmasked areas

        float maskn = clamp(texture2D(Mask, uv).r, 0.,1.);

        blurCol += texture2D(colorMap, uv).rgb * weights[j]*maskn;
        uv += dir_corrected;
        sumweights += (weights[j]*maskn);
    }
    blurCol /= sumweights;

    vec3 finalCol = mix(baseCol, blurCol, ratio*maskval);
    outColor = vec4(finalCol,1.);

}
