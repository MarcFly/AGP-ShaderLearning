#version 330 core

uniform vec4 camParams; // l r b t
uniform float znear;
uniform mat4 worldMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

in vec2 texCoord;
out vec4 outColor;

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

    // Plane Parameters
    vec3 N = vec3(0.,-1.,0.);
    vec3 P = vec3(0.);

    // RayPlane Intersec
    float num = dot(P - camPosWorldSpace, N);
    float den = dot(camDirWorldSpace, N);
    float t = num / den;

    if(t > 0.)
    {
         vec3 hit = camPosWorldSpace + camDirWorldSpace * t;
         vec4 hitClip = projectionMatrix * viewMatrix * vec4(hit , 1.);
         float g = grid(hit,100.);
         if(g == 0)
             g = grid(hit, 10.) * .7;
         if(g == 0)
             g = grid(hit, 1.) * .2;
         float ndcDepth = hitClip.z / hitClip.w;
         gl_FragDepth = ((gl_DepthRange.diff * ndcDepth) + gl_DepthRange.near + gl_DepthRange.far) / 2.;

         // Make it more black over distance
         // value is hardcoded for now

         float col = clamp(g / (t / 100.), .1, 1.);

         if(g == 0.)
         {
             gl_FragDepth = 1.;
             col = .05;
             //discard;
         }

         outColor = vec4(vec3(col), 1.);
    }
    else
    {
        gl_FragDepth = 1.;


        //discard;
        outColor = vec4(vec3(.1), 1.);

        // Do background
    }
}
