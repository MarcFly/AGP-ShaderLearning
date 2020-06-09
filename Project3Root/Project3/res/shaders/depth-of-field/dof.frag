#version 330 core

out vec4 outColor;

uniform float FocusDist;
uniform float FocusDepth;
uniform float FocusFalloff;
uniform float zfar;

uniform vec2 viewP;
uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;
uniform vec3 camPos;

uniform sampler2D depth;

void main()
{


    // Set Values to ndc coords as gl_FragDepth

    vec4 ndcDistP = (projectionMatrix * viewMatrix * vec4(0.,0.,FocusDist, 1.));
    float ndcDist = (ndcDistP / ndcDistP.w).z;
    vec4 ndcDepthP = (projectionMatrix * viewMatrix * vec4(0.,0.,FocusDepth, 1.));
    float ndcDepth = (ndcDepthP / ndcDepthP.w).z;
    vec4 ndcFalloffP = (projectionMatrix * viewMatrix * vec4(0.,0.,FocusFalloff, 1.));
    float ndcFalloff = (ndcFalloffP / ndcFalloffP.w).z;

    float focusMax = (ndcDepth*(.5) + ndcDist );
    float focusMin = (ndcDepth*(-.5) + ndcDist);

    vec2 tCoord = gl_FragCoord.xy / viewP;

    float z = texture2D(depth, tCoord).r;

    if(z > focusMin && z < focusMax) discard;

    z = clamp((abs(ndcDist - z) - ndcDepth*.5) / FocusFalloff, 0., 1.);

    outColor = vec4(z);


}
