layout(location=0) in vec3 position;

uniform mat4 worldMatrix;
uniform mat4 projectionMatrix;
uniform mat4 worldViewMatrix;

uniform vec3 lightPosition;

out float ndc_dist;
out vec2 lightPos;

void main(void)
{
    vec4 lightPosGL = projectionMatrix * worldViewMatrix * vec4(lightPosition, 1.);
    gl_Position = projectionMatrix * worldViewMatrix * vec4(position, 1.);

    lightPos = (lightPosition.xyz/lightPosition.w).xy;
    ndc_dist = length(lightPosGL.xyz / lightPosGL.w - gl_Position.xyz / gl_Position.w);
}
