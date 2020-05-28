#version 330 core

layout(location=0) out vec3 outColor;

uniform vec2 ViewPort;
uniform sampler2D depth;

in vec3 vNormal;

void main()
{
    vec2 texCoord = gl_FragCoord.xy / ViewPort;

    if(texture(depth, texCoord).r > gl_FragCoord.z) discard;

    outColor = normalize(vNormal);
}
