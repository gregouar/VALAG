#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (constant_id = 0) const float radius     = 0;
layout (constant_id = 1) const bool  vertical   = false;

layout (binding = 0) uniform sampler2D srcSampler;

layout (location = 0) in vec2 inUV;

layout (location = 0) out vec4 outColor;


void main()
{
    vec2 offset = vec2(1-int(vertical), int(vertical)) * radius;

    outColor =     (texture(srcSampler, inUV/*gl_FragCoord.xy*/ + offset * 1.0)	* 0.06 +
                    texture(srcSampler, inUV/*gl_FragCoord.xy*/ + offset * 0.75)	* 0.09 +
                    texture(srcSampler, inUV/*gl_FragCoord.xy*/ + offset * 0.5)	* 0.12 +
                    texture(srcSampler, inUV/*gl_FragCoord.xy*/ + offset * 0.25)	* 0.15 +
                    texture(srcSampler, inUV/*gl_FragCoord.xy*/)	* 0.16 +
                    texture(srcSampler, inUV/*gl_FragCoord.xy*/ - offset * 1.0) 	* 0.06 +
                    texture(srcSampler, inUV/*gl_FragCoord.xy*/ - offset * 0.75)	* 0.09 +
                    texture(srcSampler, inUV/*gl_FragCoord.xy*/ - offset * 0.5)	* 0.12 +
                    texture(srcSampler, inUV/*gl_FragCoord.xy*/ - offset * 0.25)	* 0.15 );
}

