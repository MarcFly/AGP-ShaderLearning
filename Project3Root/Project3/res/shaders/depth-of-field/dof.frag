#version 330 core

layout(location=0) out float outColor;

uniform float FocusDist;
uniform float FocusDepth;
uniform float FocusFalloff;
uniform float zfar;

uniform vec2 viewP;
uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;
uniform vec3 camPos;

uniform sampler2D depth;

void main()
{
    float diff = (gl_DepthRange.far - gl_DepthRange.near);
    float cDist = FocusDist / diff;
    float cDepth = FocusDepth / diff;
    float cFalloff = FocusFalloff / diff;

    vec2 texCoord = gl_FragCoord.xy / viewP;
    float texel = texture2D(depth, texCoord).r;

    float f = 10000.0;
    float n = 0.01;
    float z = abs((2 * f * n) / ((texel * 2.0 - 1.0) *(f-n)-(f+n)));
    z = ((z / 50.0));
    //outColor = z;
    outColor = (max(0, z - cDist - cDepth) / ((cDist + (cDepth /2.)+cFalloff) - cDepth))*diff;

    //outColor *= 1000000.;
    //outColor = 1.;

//*/

}
