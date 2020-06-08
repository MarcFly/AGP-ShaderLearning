#version 330 core

layout(location=0) out float WriteMask;

uniform float FocusDist;
uniform float FocusDepth;
uniform float FocusFalloff;
uniform float zfar;

uniform vec2 viewP;
uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;
uniform vec3 camPos;

void main()
{
    float focusMax = (FocusDepth*(.5) + FocusDist ) / zfar;
    float focusMin = (FocusDepth*(-.5) + FocusDist) / zfar;

    float d = gl_FragDepth * 2. - 1.;
    vec2 tCoord = gl_FragCoord.xy / viewP;
    vec4 clipSpace = vec4(tCoord * 2. - 1., d, 1.);
    vec4 viewSpace = inverse(projectionMatrix) * clipSpace;
    viewSpace /= viewSpace.w;

    vec4 worldSpace = inverse(viewMatrix) * viewSpace;

    float dist = length(worldSpace.xyz - camPos);

    if(dist > focusMin && dist < focusMax) discard;

    // Linear Falloff of focus
    WriteMask = clamp(abs(dist - FocusDist) / (FocusFalloff), 0., 1.);


}
