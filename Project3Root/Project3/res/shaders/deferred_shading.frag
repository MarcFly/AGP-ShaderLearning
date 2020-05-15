#version 330 core

// Light Data
#define MAX_LIGHTS 8
uniform int lightType;
uniform vec3 lightPosition;
uniform vec3 lightDirection;
uniform vec3 lightColor;
uniform float lightIntensity;
uniform float lightRange;
uniform float Kc;
uniform float Kl;
uniform float Kq;
uniform mat4 worldMatrix;
uniform vec3 camPos;

uniform sampler2D gboPosition;
uniform sampler2D gboNormal;
uniform sampler2D gboAlbedoSpec;
uniform vec2 ViewPort;
out vec4 outColor;

#define MIN (5. / 256.)

uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;
uniform mat4 worldViewMatrix;

void main(void)
{
    // Prep Values for Calc
    vec2 texCoord = gl_FragCoord.xy / ViewPort;
    vec3 rPos = texture(gboPosition, texCoord).rgb;
    vec3 N = texture(gboNormal, texCoord).rgb;
    vec4 albedoSpec = texture(gboAlbedoSpec, texCoord).rgba;
    vec3 camVec = normalize(rPos - camPos);

    // Calculate Attenuation
    // https://learnopengl.com/Lighting/Light-casters
    float dist = distance(lightPosition, rPos) * 1.3;
    float attenuation = 1. / (Kq*pow(dist, 2.) + Kl*dist + Kc);

    float cutre = smoothstep(lightRange, 0, length(rPos - lightPosition));
    attenuation = cutre;

    vec4 glPos = vec4(gl_FragCoord.xyz / gl_FragCoord.w,1./(1./gl_FragCoord.w));
    vec3 realPos =  (inverse(projectionMatrix) * inverse(viewMatrix) * glPos).xyz;

    //attenuation = smoothstep(lightRange, 0, length(realPos - lightPosition));

    // Diffuse - Lambertian
    // Specular - Fresnel, the more parallel the viewer is to a surface, more reflective it becomes

    vec3 finalCol;

    vec3 ldir = lightDirection;
    if(lightType == 0)
        ldir = rPos - lightPosition;
    ldir = normalize(-ldir);

    // Our definition of Diffuse, is basically what is really called Lambertian Reflection
    float k_d = max(dot(ldir,N), 0.);
    finalCol.rgb += albedoSpec.rgb * k_d * lightColor.rgb;

    // We need to pass camera position or direction to get specular ffs
    // Blin = Halfway direction between CameraVector and Light Direction
    vec3 BlinVec = normalize(camVec + ldir);
    // Shininess factor needs to be passed from Material, not just an arbitrary value.
    float k_s = pow(max(dot(N, BlinVec), 0.), 2.);

    // Phong = Get ldir that reflects off of Normal of the Point
    vec3 PhongVec = normalize(reflect(ldir, N));
    //float k_s = pow(max(dot(camVec, PhongVec), 0.), 16.);

    finalCol.rgb += albedoSpec.rgb * lightColor.rgb * (albedoSpec.a * k_s);

    outColor = vec4(finalCol.rgb,1.) * lightIntensity * attenuation;
}
