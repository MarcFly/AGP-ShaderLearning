#version 330 core

uniform vec3 SelectionCode;

out vec4 outColor;
void main(void)
{
    outColor = vec4(SelectionCode,1.0);
}
