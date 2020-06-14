#version 330 core

layout(location=0) out vec4 outColor;
layout(location=1) out vec4 albedoSpec;

uniform sampler2D depth;
uniform sampler2D normal;

uniform vec3[64] kernel;
uniform vec3[16] noise;

uniform float aoRad;

uniform mat4 projection;

// Camera Parameters
uniform vec4 camParams;
uniform vec2 z;

in vec2 texCoord;

vec3 pixelPos(float d, float l, float r, float b, float t, float n, float f, vec2 v)
{
    float zndc = d * 2. - 1.;
    float zeye = 2*f*n / (zndc*(f-n)-(f+n));
    float xndc = gl_FragCoord.x/v.x * 2. - 1.;
    float yndc = gl_FragCoord.y/v.y * 2. - 1.;
    float xeye = -zeye*((xndc)*(r-l))/(2.*n);
    float yeye = -zeye*(yndc*(t-b)+(t+b))/(2.*n);
    return vec3(xeye, yeye, zeye);

}


vec3 fpixelPos(float d, mat4 invPM, vec2 v)
{
    float xndc = gl_FragCoord.x/v.x *2. - 1.;
    float yndc = gl_FragCoord.y/v.y *2. - 1.;
    float zndc = d * 2. - 1.;
    vec4 posNDC = vec4(xndc, yndc, zndc, 1.);
    vec4 posView = invPM *posNDC;
    return posView.xyz / posView.w;
}

void main()
{
    vec2 vpSize = textureSize(depth,0);

    float d = texture2D(depth, texCoord).x;
    vec3 fragPos = fpixelPos(d, inverse(projection), vpSize);
    vec3 normal = texture2D(normal, texCoord).rgb;

    int nval = int(texCoord * 15);
    vec3 rvec = noise[nval];

    vec3 tangent = normalize(rvec - normal * dot(rvec, normal));
    vec3 bitangent = cross(normal, tangent);
    mat3 TBN = mat3(tangent, bitangent, normal);

    float occl = 0.;
    for(int i = 0;i < 64; ++i)
    {
        vec3 smpl = TBN * kernel[i];
        smpl = fragPos + smpl*aoRad;

        vec4 offset = vec4(smpl, 1.);
        offset = projection * offset;
        offset.xyz /= offset.w;
        offset.xyz = offset.xyz *.5 + .5;
        float smplDepth = texture(depth, offset.xy).x;

        float rangeCheck = smoothstep(0.,1., aoRad / abs(d - smplDepth));
        occl += (smplDepth >=d + .001 ? 1. : 0.)*rangeCheck;
    }


    // End Color
    outColor = vec4(1. - (occl / 64));
    albedoSpec = vec4(1.);
}
