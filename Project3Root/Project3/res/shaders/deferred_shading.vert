#version 330 core

layout(location=0) in vec3 position;
layout(location=2) in vec2 texCoords;
uniform mat4 projectionMatrix;
uniform mat4 worldViewMatrix;
uniform vec2 ViewPort;

out vec2 texCoord;

void main(void)
{
    gl_Position = projectionMatrix * worldViewMatrix * vec4(position, 1.);

    texCoord = (((gl_Position.xyz / gl_Position.w) * .5) + .5).xy;
    //texCoord = (floor(texCoord) + .5);

    //texCoord = texCoords;
    //gl_Position = vec4(position, 1.0);
}
