#version 330 core

layout(location=0) out float outColor;

//out float outColor;

uniform float FocusDist;
uniform float FocusDepth;
uniform float FocusFalloff;
uniform float zfar;
uniform float znear;

uniform vec2 viewP;

uniform sampler2D depth;

void main()
{
    float diff = (zfar - znear);
    float cDist = FocusDist / diff;
    float cDepth = FocusDepth / diff;
    float cFalloff = FocusFalloff / diff;

    vec2 texCoord = gl_FragCoord.xy / viewP;
    float texel = texture2D(depth, texCoord).r;

    float f = zfar;
    float n = znear;
    float z = abs((2 * f * n) / ((texel * 2.0 - 1.0) *(f-n)-(f+n)));
    z = ((z / 50.0));
    //outColor = z;
    outColor = (max(0, z - cDist - cDepth) / ((cDist + (cDepth /2.)+cFalloff) - cDepth))*diff;

    outColor = 1.;

}
