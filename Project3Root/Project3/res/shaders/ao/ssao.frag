#version 330 core

out float outColor;

uniform sampler2D depth;
uniform sampler2D normal;
uniform sampler2D noiseTex;

uniform vec3[64] kernel;

uniform float aoRad;

uniform mat4 projection;

// Camera Parameters
uniform vec4 camParams;
uniform vec2 z;
uniform vec2 viewP;

in vec2 texCoord;

float rand(vec2 n)
{
    return fract(sin(dot(n, vec2(12.9898, 4.1414))) * (43758.5453));
}

vec3 offsetPixelPos(float d, float l, float r, float b, float t, float n, float f, vec2 v)
{
    float zndc = d * 2. -1.;
    float zeye = 2*f*n / (zndc*(f-n)-(f+n));
    float xndc = gl_FragCoord.x / v.x * 2. - 1.;
    float yndc = gl_FragCoord.y / v.y * 2. - 1.;
    float xeye = -zeye*(xndc*(r-l)+(r+l))/(2.*n);
    float yeye = -zeye*(yndc*(r-l)+(r+l))/(2.*n);

    return vec3(xeye, yeye, zeye);
}

vec3 depthPixelPos(float d, mat4 invPM, vec2 v)
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
    float d = texture2D(depth, texCoord).x;
    vec3 fragPos = depthPixelPos(d, inverse(projection), viewP);
    vec3 vnormal = texture2D(normal, vec2(rand(texCoord))).rgb;

    vec2 nScale = viewP / 4;
    vec3 rvec = texture2D(noiseTex, texCoord/viewP).rgb;

    vec3 tangent = normalize(rvec - vnormal * dot(rvec, vnormal));
    vec3 bitangent = cross(vnormal, tangent);
    mat3 TBN = mat3(tangent, bitangent, vnormal);

    float occl = 0.;
    for(int i = 0;i < 64; ++i)
    {
        // Generate vectors in real space
        // TBN transforms V into Normal Space
        // THen this smpl vec is applied to position
        // put at radius area
        vec3 smpl = TBN * kernel[i];
        smpl = fragPos + smpl*aoRad;

        // Generate a vec2 in texture space
        // The vector in world space is put in
        // clip space to get a depth value from the depthBuffer
        vec4 offset = vec4(smpl, 1.);
        offset = projection * offset;
        offset.xyz /= offset.w;
        offset.xyz = offset.xyz *.5 + .5;
        float d2 = texture(depth, offset.xy).x;

        // Transform depth into World Pos
        vec3 smplpos = offsetPixelPos(d2, camParams.x, camParams.y, camParams.z, camParams.w, z.x, z.y, viewP);

        // Occlusion test is done by seeing if the z difference(in tangent space)
        // is or not behind the actual geometry
        // In addition we test if the distance is greater than it should
        // thus making sure that distances too big don't occlude each other
        float rangeCheck = smoothstep(1., 0., aoRad / abs(smplpos.z - smpl.z));
        rangeCheck *= rangeCheck;
        occl += (smplpos.z < smpl.z - .02 ? 1.:0.) * rangeCheck;


    }


    // End Color
    outColor = 1. - (occl / 64);
    //outColor = 1.;
}
