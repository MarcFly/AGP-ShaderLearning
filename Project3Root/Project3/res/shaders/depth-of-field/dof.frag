#version 330 core

layout(location=0) out float WriteMask;

uniform float FocusDist;
uniform float FocusDepth;
uniform float FocusFalloff;
uniform float zfar;

void main()
{
    float focusMax = (FocusDepth*(.5) + FocusDist ) / zfar;
    float focusMin = (FocusDepth*(-.5) + FocusDist) / zfar;

    if(gl_FragDepth > focusMin && gl_FragDepth < focusMax) discard;

    // Linear Falloff of focus
    WriteMask = clamp(abs(gl_FragDepth - FocusDist / zfar) / (FocusFalloff / zfar), 0., 1.);
}
