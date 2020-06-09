#version 330 core

out vec4 outColor;

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

    outColor = vec4(max(0, gl_FragDepth - cDist - cDepth) / ((cDist + (cDepth /2.)+cFalloff) - cDepth));

}
