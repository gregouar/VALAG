#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (binding = 0) uniform sampler2D samplerHdr;

layout (location = 0) in vec2 inUV;

layout (location = 0) out vec4 outColor;

void main()
{
    float exposure = 0.8;
    outColor = vec4(vec3(1.0) - exp(-texture(samplerHdr,inUV).rgb * exposure), 1.0);
    outColor = vec4(0.0,1.0,0.0,1.0);
}

