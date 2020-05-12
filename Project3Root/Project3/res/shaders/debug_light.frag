#version 330 core

// Light Data
uniform vec3 lightColor;
uniform float lightIntensity;
uniform float lightRange;
uniform float Kc;
uniform float Kl;
uniform float Kq;

in float ndc_dist;
in vec2 lightPos;

out vec4 outColor;

void main(void)
{

    float realDist = length(lightPos - gl_FragCoord.xy); // / (lightRange / ndc_dist);
    float attenuation = 1. / (Kq*pow(realDist, 2.) + Kl*realDist + Kc);

    attenuation *= 5.;
    outColor = vec4(lightColor * lightIntensity / attenuation, 1.);

    float smooth_att = smoothstep(lightRange, 0., realDist);

    //outColor = vec4(lightColor * smooth_att , 1.);

    //outColor = vec4(0.);
}
