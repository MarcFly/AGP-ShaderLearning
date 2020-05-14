layout(location=0) in vec3 position;

uniform mat4 worldMatrix;
uniform mat4 projectionMatrix;
uniform mat4 worldViewMatrix;

uniform vec3 lightPosition;
uniform mat4 LimitDisplace;
uniform float lightRange;

out float ndc_dist;
out vec2 lightPos;
out float ratio;

void main(void)
{
    gl_Position = projectionMatrix * worldViewMatrix * vec4(position, 1.);

    vec3 newLimit = (limitDisplace * vec4(0.,0.,1.,1.)).xyz * lightRange + lightPos;
    vec4 lightLimit =  projectionMatrix * worldViewMatrix * vec4(newLimit, 1.);
    vec2 limit2D = (lightLimit.xyz / lightLimit.w).xy;

    vec4 lightPosGL = projectionMatrix * worldViewMatrix * vec4(lightPosition, 1.);
    lightPos = (lightPosGL.xyz/lightPosGL.w).xy;

    vec3 lightPos3D = lightPosGL.xyz;
    vec3 limit3D  = ligtLimit.xyz;

    ratio = lightRange / length(lightPos - limit2D);

}
