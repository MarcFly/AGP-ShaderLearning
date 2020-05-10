#version 330 core

// Matrices
uniform mat4 worldViewMatrix;
uniform mat4 worldMatrix;
uniform mat3 normalMatrix;

// Material
uniform vec4 albedo;
uniform vec4 specular;
uniform vec4 emissive;
uniform float smoothness;
uniform float bumpiness;
uniform sampler2D albedoTexture;
uniform sampler2D specularTexture;
uniform sampler2D emissiveTexture;
uniform sampler2D normalTexture;
uniform sampler2D bumpTexture;

// Lights
#define MAX_LIGHTS 8
uniform int lightType;
uniform vec3 lightPosition;
uniform vec3 lightDirection;
uniform vec3 lightColor;
uniform float lightRange;
uniform float Kc;
uniform float Kl;
uniform float Kq;

uniform vec3 camPos;

uniform sampler2D gboPosition;
uniform sampler2D gboNormal;
uniform sampler2D gboAlbedoSpec;

out vec4 outColor;
in vec2 texCoord;

#define AMBIENT .5
#define MIN (8. / 256.)

void main(void)
{
    // Prep vals
    vec3 rPos = texture(gboPosition, texCoord).rgb;

    // Calculate Attenuation
    // https://learnopengl.com/Lighting/Light-casters
    float dist = length(rPos - lightPosition);
    float att = 1. / (Kc + dist * Kl + Kq*pow(dist, 2));
    float light_max = max(max(lightColor.r, lightColor.g), lightColor.b);
    float min_dist = (sqrt(pow(Kl,2.) - 4.*Kq*((-1./MIN)* light_max) + Kc)-Kl) / (2.*Kq);
    if( dist > min_dist)
        discard;

    vec3 N = texture(gboNormal, texCoord).rgb;
    vec4 albedoSpec = texture(gboAlbedoSpec, texCoord).rgba;

    vec3 camVec = normalize(rPos - camPos);

    // TODO: Local illumination
    // Ambient
    // Diffuse - Lambertian
    // Specular - Fresnel, the more parallel the viewer is to a surface, more reflective it becomes
    vec3 finalCol = albedoSpec.rgb * AMBIENT;

    vec3 ldir = lightDirection;
    if(lightType == 0)
        ldir = rPos - lightPosition;
    ldir = normalize((-1.) * ldir);

    // Our definition of Diffuse, is basically what is realyl called Lambertian Reflection
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

    outColor = vec4(finalCol.rgb,1.) * att;
}
