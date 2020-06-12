#version 330 core

layout (location = 0) out vec4 outColor;
layout (location = 1) out vec3 gboPosition;
layout (location = 2) out vec3 gboNormal;
layout (location = 3) out vec4 gboAlbedoSpec;
layout (location = 4) out vec3 fboAmbient;

in vec2 vTexCoords;
in vec3 vNormal;
in vec3 rPos;

uniform vec4 albedo;
uniform vec4 specular;
uniform sampler2D albedoTexture;
uniform sampler2D specularTexture;

uniform float AMBIENT;

void main()
{
    gboPosition = rPos;

    gboNormal = normalize(vNormal);

    vec4 albedoVal = albedo.rgba * texture(albedoTexture, vTexCoords).rgba;
    vec4 specVal = specular.rgba * texture(specularTexture, vTexCoords).rgba;
    gboAlbedoSpec = vec4(albedoVal.rgb, specVal.a);
    fboAmbient = vec3(albedoVal.rgb);

    outColor = vec4(fboAmbient * AMBIENT, 1.);
}
