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
uniform int lightType[MAX_LIGHTS];
uniform vec3 lightPosition[MAX_LIGHTS];
uniform vec3 lightDirection[MAX_LIGHTS];
uniform vec3 lightColor[MAX_LIGHTS];
uniform int lightCount;
uniform vec3 camPos;

uniform sampler2D gboPosition;
uniform sampler2D gboNormal;
uniform sampler2D gboAlbedoSpec;

out vec4 outColor;
in vec2 texCoord;

#define AMBIENT .05

void main(void)
{
    // Prep vals
    vec3 rPos = texture(gboPosition, texCoord).rgb;
    vec3 N = texture(gboNormal, texCoord).rgb;
    vec4 albedoSpec = texture(gboAlbedoSpec, texCoord).rgba;

    vec3 camVec = normalize(rPos - camPos);

    // TODO: Local illumination
    // Ambient
    // Diffuse
    // Specular
    vec3 finalCol = albedoSpec.rgb * AMBIENT;

    for(int i = 0; i < lightCount; i++)
    {

        vec3 ldir = lightDirection[i];
        if(lightType[i] == 0)
            ldir = rPos - lightPosition[i];
        ldir = normalize((-1.) * ldir);

        float k_d = max(dot(ldir,N), 0.);
        finalCol.rgb += albedoSpec.rgb * k_d * lightColor[i].rgb;

        // We need to pass camera position or direction to get specular ffs
        // Blin = Halfway direction between CameraVector and Light Direction
        vec3 BlinVec = normalize(camVec + ldir);
        // Shininess factor needs to be passed from Material, not just 16.
        float k_s = pow(max(dot(N, BlinVec), 0.), 2.);

        // Phong = Get ldir that reflects off of Normal of the Point
        vec3 PhongVec = normalize(reflect(ldir, N));
        //float k_s = pow(max(dot(camVec, PhongVec), 0.), 16.);

        finalCol.rgb += albedoSpec.rgb * lightColor[i].rgb * (albedoSpec.a * k_s);

    }


    outColor.rgb = finalCol.rgb;
}
