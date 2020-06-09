#version 330 core

layout(location=0) out float outColor;

//out float outColor;

out vec4 outC;


uniform float FocusDist;
uniform float FocusDepth;
uniform float FocusFalloff;
uniform float zfar;
uniform float znear;

uniform vec2 viewP;

uniform sampler2D depth;

void main()
{
    gl_FragDepth = 0.;
    outC = vec4(0.);
    outColor = 0.;
}
