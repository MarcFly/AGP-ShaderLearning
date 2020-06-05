#version 330 core

uniform float SelectionCode;

out vec4 outColor;
void main(void)
{
    outColor = vec4(vec3(SelectionCode),1.0);
}
