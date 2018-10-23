#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0, set = 0) uniform ViewUBO {
    mat4 view;
    mat4 viewInv;
    vec2 screenOffset;
    vec2 screenSizeFactor;
    vec2 depthOffsetAndFactor;
} viewUbo;

layout(location = 0) in vec3 inDirection;
layout(location = 1) in vec3 inSize;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in vec2 inTexExtent;
layout(location = 4) in uvec2 inAlbedoTexId;
layout(location = 5) in uvec2 inHeightTexId;

layout(location = 0) flat   out vec3 viewLightDirection;
layout(location = 1)        out vec2 outUV;
layout(location = 2)        out vec2 spriteTexCoord;
layout(location = 3)        out vec2 spriteTexExtent;
layout(location = 4) flat   out uvec2 spriteAlbedoTexId;
layout(location = 5) flat   out uvec2 spriteHeightTexId;
layout(location = 6)        out vec2 spritePos;
layout(location = 7)        out vec3 spriteSize;


out gl_PerVertex
{
	vec4 gl_Position;
};

void main()
{
	outUV = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2);
	gl_Position = vec4(outUV * 2.0f - 1.0f, 0.0f, 1.0f);


	/*vec3 lightDirection = normalize(inDirection);
	viewLightDirection.xy = (viewUbo.view * vec4(lightDirection.xy / -lightDirection.z, 0.0, 0.0)).xy;
	viewLightDirection.xy = normalize(viewLightDirection.xy * inSize.xy)*0.5;
	viewLightDirection.z = lightDirection.z;
    spritePos.xy        = max(vec2(0.0), -viewLightDirection.xy);
    spriteSize.xy       = vec2(1.0) - abs(viewLightDirection.xy);
    spriteSize.z        = inSize.z;*/


    vec3 lightDirection = normalize(inDirection);
	viewLightDirection.xy = (viewUbo.view * vec4(inSize.z*lightDirection.xy / -lightDirection.z, 0.0, 0.0)).xy;
	viewLightDirection.y -= inSize.z * viewUbo.view[2][1];
	vec2 totalSize        = abs(viewLightDirection.xy)+inSize.xy;

	viewLightDirection.xy = viewLightDirection.xy/totalSize;
	viewLightDirection.z  = -1.0;//-inSize.z;//lightDirection.z;

    spritePos.xy        = max(vec2(0.0), -viewLightDirection.xy);
    spriteSize.xy       = vec2(1.0) - abs(viewLightDirection.xy);
    spriteSize.z        = inSize.z;

	spriteTexCoord      = inTexCoord;
	spriteTexExtent     = inTexExtent;
    spriteAlbedoTexId   = inAlbedoTexId;
    spriteHeightTexId   = inHeightTexId;

}


