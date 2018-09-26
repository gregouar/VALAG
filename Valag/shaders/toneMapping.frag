#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (binding = 0) uniform sampler2D samplerHdr;
layout (binding = 1) uniform sampler2D samplerAlphaHdr;

layout (location = 0) in vec2 inUV;

layout (location = 0) out vec4 outColor;


void main()
{
    float exposure = 0.8;

    vec3 opac   = texture(samplerHdr,gl_FragCoord.xy).rgb;
    vec4 alpha  = texture(samplerAlphaHdr,gl_FragCoord.xy);

    vec3 final  = opac * (1.0f-alpha.a) + alpha.rgb * alpha.a;

    outColor = vec4(vec3(1.0) - exp(-final * exposure), 1.0);
    outColor.rgb = pow(outColor.rgb, vec3(1.0/2.2));
}

