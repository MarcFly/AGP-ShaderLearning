#version 330 core
layout(location=0) out vec4 outColor;
uniform sampler2D mask;
uniform sampler2D inColor;

uniform vec2 viewP;
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

    // Double Pass
    vec3 blurredCol= vec3(0.0);
    //vec2 texCoords = gl_FragCoord.xy / viewP;

    float val = texture2D(mask, texCoord).r;
    vec3 baseCol = texture2D(inColor, texCoord).rgb;

    //if(val < .01) discard;

    for(int i = 0; i < 11; ++i)
    {
        float sumweights = 0.;
        vec3 stepCols = vec3(0.);
        for(int j = 0; j < 11; ++j)
        {
            vec2 uv = texCoord + vec2(i, j);
            stepCols += texture2D(inColor, uv).rgb * weights[j];
            sumweights += weights[j];

        }
        stepCols /= sumweights;
        blurredCol += stepCols;
    }

    vec3 finalCol = mix(baseCol, blurredCol, ratio);
    outColor = vec4(blurredCol,1.);

}
