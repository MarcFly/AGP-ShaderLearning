#version 330 core

uniform sampler2D ssaoMask;
uniform sampler2D albedo;

in vec2 texCoord;

out vec4 outColor;

uniform float AMBIENT;

void main()
{
    outColor = texture2D(albedo, texCoord).rgba * texture2D(ssaoMask, texCoord).r * AMBIENT;
}
