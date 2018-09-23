#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (binding = 0) uniform sampler2D samplerAlbedo;
layout (binding = 1) uniform sampler2D samplerPosition;
layout (binding = 2) uniform sampler2D samplerNormal;
layout (binding = 3) uniform sampler2D samplerRmt;

layout (binding = 4) uniform sampler2D samplerAlphaAlbedo;
layout (binding = 5) uniform sampler2D samplerAlphaPosition;
layout (binding = 6) uniform sampler2D samplerAlphaNormal;
layout (binding = 7) uniform sampler2D samplerAlphaRmt;

layout (location = 0) in vec2 inUV;

layout (location = 0) out vec4 outColor;
layout (location = 1) out vec4 outAlphaColor;

void main()
{
    outColor = texture(samplerAlbedo, gl_FragCoord.xy);
    outAlphaColor = texture(samplerAlphaAlbedo, gl_FragCoord.xy);
}

