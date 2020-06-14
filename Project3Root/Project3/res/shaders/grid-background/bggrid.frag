#version 330 core

uniform vec4 camParams; // l r b t
uniform float znear;
uniform mat4 worldMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

uniform vec3 bgCol;
uniform vec3 gridCol;

in vec2 texCoord;
out vec4 outColor;

uniform sampler2D depth;

float grid(vec3 worldPos, float gridStep)
{
    vec2 grid = fwidth(worldPos.xz);
    grid = grid / mod(worldPos.xz, gridStep);
    float line = step(1., max(grid.x, grid.y));
    return line;
}

void main(void)
{
    outColor = vec4(1.,1.,1.,1.);

    // Get Camera Direciton
    vec3 camDir;
    camDir.x = camParams.x + texCoord.x * (camParams.y - camParams.x);
    camDir.y = camParams.z + texCoord.y * (camParams.w - camParams.z);
    camDir.z = -znear;
    vec3 camDirWorldSpace = normalize(mat3(worldMatrix) * camDir);
    // Cam Position
    vec3 camPos = vec3(0.);
    vec3 camPosWorldSpace = vec3(worldMatrix * vec4(camDir, 1.));

    float d = texture2D(depth, texCoord).r;

    // Plane Parameters
    vec3 N = vec3(0.,-1.,0.);
    vec3 P = vec3(0.);

    // RayPlane Intersec
    float num = dot(P - camPosWorldSpace, N);
    float den = dot(camDirWorldSpace, N);
    float t = num / den;

    if(t > 0.)
    {
        vec3 hit = camPosWorldSpace + camDirWorldSpace * abs(t);
        vec4 hitClip = projectionMatrix * viewMatrix * vec4(hit , 1.);
        float g = grid(hit,100.)*.4 + grid(hit, 10.)*.3 + grid(hit, 1.)*.3;
        g = clamp(g, 0., 1.);

        float ndcDepth = hitClip.z / hitClip.w;
        gl_FragDepth = ((gl_DepthRange.diff * ndcDepth) + gl_DepthRange.near + gl_DepthRange.far) / 2.;

         // g is 0 or gridval depending on if the previous depth is closer
         g  *= (gl_FragDepth < d) ? 1. : 0.;

         float bg_alpha = (1. - g) * ((1. > d) ? 0. : 1.);

         // Alpha is distance based
         float dist_alpha  = clamp(1. / (t / 200.), 0., 1.);

         // Here we have basically 0 if no grid or object before
         // but, no grid should have X color with alpha 1
         // object before should have alpha 0
         vec3 col = mix(gridCol, bgCol, 1. - dist_alpha);
         outColor = vec4(g*col + (1.-g)*bgCol, g+bg_alpha);
    }
    else
    {
        outColor = vec4(bgCol, (d < 1.) ? 0.:1.);
    }
}
