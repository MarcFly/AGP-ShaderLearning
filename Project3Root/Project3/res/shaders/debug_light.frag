#version 330 core

// Light Data
uniform vec3 lightColor;
uniform float lightIntensity;
uniform float lightRange;
uniform float Kc;
uniform float Kl;
uniform float Kq;

uniform vec2 ViewPort;

in float ndc_dist;
in vec2 lightPos;
in float ratio;

out vec4 outColor;

void main(void)
{

    float realDist = length(lightPos - (gl_FragCoord.xy / ViewPort)) * ratio; // / (lightRange / ndc_dist);
    float attenuation = 1. / (Kq*pow(realDist, 2.) + Kl*realDist + Kc);

    //attenuation *= 5.;
    //outColor = vec4(lightColor * lightIntensity * attenuation, 1.);

    float smooth_att = smoothstep(lightRange, 0., realDist);

    //outColor = vec4(lightColor * smooth_att , 1.);

    //outColor = vec4(0.);

    outColor = vec4(vec3(attenuation), 1.);
}
