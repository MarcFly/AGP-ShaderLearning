#version 330 core

// Light Data
#define MAX_LIGHTS 8
uniform int lightType;
uniform vec3 lightPosition;
uniform vec3 lightDirection;
uniform vec3 lightColor;
uniform float lightIntensity;
uniform float lightRange;
uniform mat4 worldMatrix;
uniform vec3 camPos;

uniform sampler2D gboPosition;
uniform sampler2D gboNormal;
uniform sampler2D gboAlbedoSpec;
uniform vec2 ViewPort;
out vec4 outColor;

void main(void)
{
    // Prep Values for Calc
    vec2 texCoord = gl_FragCoord.xy / ViewPort.xy;
    vec3 rPos = texture2D(gboPosition, texCoord).rgb;
    vec3 N = texture2D(gboNormal, texCoord).rgb;
    vec4 albedoSpec = texture2D(gboAlbedoSpec, texCoord).rgba;
    vec3 camVec = normalize(rPos - camPos);

    // Testing
    // Calculate Attenuation
    // https://learnopengl.com/Lighting/Light-casters

    float dist = length(rPos - lightPosition);
    float attenuation = 1. - smoothstep(0., lightRange, dist);

    // Diffuse - Lambertian
    // Specular - Fresnel, the more parallel the viewer is to a surface, more reflective it becomes

    vec3 finalCol = vec3(0.);

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

    outColor = vec4(finalCol.rgb, attenuation * lightIntensity);
}
