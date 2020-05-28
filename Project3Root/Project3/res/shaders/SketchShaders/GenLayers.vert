#version 330 core

layout(location=0) in vec3 position;
layout(location=1) in vec3 normal;

uniform mat4 projectionMatrix;
uniform mat4 worldViewMatrix;
uniform mat4 worldMatrix;

out vec3 vNormal;

void main(void)
{
    gl_Position = projectionMatrix * worldViewMatrix * vec4(position, 1.);

    vNormal = (worldMatrix * vec4(normal, 0.)).xyz;
}
