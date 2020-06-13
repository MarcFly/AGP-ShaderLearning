#version 330 core

layout(location=0) out vec4 outColor;

uniform sampler2D mask;

uniform float alpha;
uniform float width;

in vec2 texCoord;

void main(void)
{
    if(width == 0. || alpha == 0.) discard;

    if(texture2D(mask, texCoord).r > 0.1) discard;

    float w = width / textureSize(mask, 0).x;
    vec3 inc = vec3(1.,0.,-1.) * w;
    vec2 incd = vec2(1./sqrt(2.), -1./sqrt(2.)) * w;

    float u = texture2D(mask, texCoord + inc.yx).r;
    float d = texture2D(mask, texCoord + inc.yz).r;
    float r = texture2D(mask, texCoord + inc.xy).r;
    float l = texture2D(mask, texCoord + inc.zy).r;

    float dur = texture2D(mask, texCoord + incd.xx).r;
    float dul = texture2D(mask, texCoord + incd.yx).r;
    float ddr = texture2D(mask, texCoord + incd.xy).r;
    float ddl = texture2D(mask, texCoord + incd.yy).r;

    bool test = u > .1 || d > .1 || r > .1 || l > .1 || dur > .1 || dul > .1 || ddr > .1 || ddl > .1;
    if(!test) discard;

    outColor = vec4(1., .5, 0., alpha);
}
