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

in vec3 vNormal;
in vec2 vTexCoords;
in vec3 rPos;
out vec4 outColor;

#define AMBIENT .05

void main(void)
{
    // Prep vals
    vec3 N = normalize(vNormal);
    vec3 camVec = normalize(rPos - camPos);

    // TODO: Local illumination
    // Ambient
    // Diffuse
    // Specular
    vec3 objCol = albedo.rgb * texture2D(albedoTexture, vTexCoords).rgb;
    vec3 finalCol = objCol * AMBIENT;

    for(int i = 0; i < lightCount; i++)
    {

        vec3 ldir = lightDirection[i];
        if(lightType[i] == 0)
            ldir = rPos - lightPosition[i];
        ldir = normalize((-1.) * ldir);

        float k_d = max(dot(ldir,N), 0.);
        finalCol.rgb += objCol.rgb * k_d * lightColor[i].rgb;

        // We need to pass camera position or direction to get specular ffs
        // Blin = Halfway direction between CameraVector and Light Direction
        vec3 BlinVec = normalize(camVec + ldir);
        // Shininess factor needs to be passed from Material, not just 16.
        //float k_s = pow(max(dot(camVec, BlinVec), 0.), 16.);

        // Phong = Get ldir that reflects off of Normal of the Point
        vec3 PhongVec = normalize(reflect(ldir, N));
        float k_s = pow(max(dot(camVec, PhongVec), 0.), 16.);

        finalCol.rgb += specular.rgb * texture2D(specularTexture, vTexCoords).rgb * k_s * lightColor[i].rgb;

    }


    outColor.rgb = finalCol.rgb;
}
