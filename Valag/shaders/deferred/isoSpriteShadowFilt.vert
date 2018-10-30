#version 450
#extension GL_ARB_separate_shader_objects : enable


layout(location = 0) in vec3 inDirection;
layout(location = 1) in vec3 inSize;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in vec2 inTexExtent;
layout(location = 4) in uvec2 inAlbedoTexId;
layout(location = 5) in uvec2 inHeightTexId;

layout(location = 0)        out vec2 outUV;
layout(location = 1) flat   out vec2 spriteSize;
layout(location = 2) flat   out uvec2 spriteAlbedoTexId;


out gl_PerVertex
{
	vec4 gl_Position;
};

void main()
{
	outUV = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2);
	gl_Position = vec4(outUV * 2.0f - 1.0f, 0.0f, 1.0f);

	spriteSize          = inSize.xy;
	spriteAlbedoTexId   = inAlbedoTexId;

	/*spriteTexCoord      = inTexCoord;
	spriteTexExtent     = inTexExtent;
    spriteAlbedoTexId   = inAlbedoTexId;
    spriteHeightTexId   = inHeightTexId;*/

}


