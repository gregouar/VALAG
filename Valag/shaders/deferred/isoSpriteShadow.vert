#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0, set = 0) uniform ViewUBO {
    mat4 view;
    mat4 viewInv;
    vec2 screenOffset;
    vec2 screenSizeFactor;
    vec2 depthOffsetAndFactor;
} viewUbo;

layout(push_constant) uniform PER_OBJECT
{
    vec4 camPosAndZoom;
}pc;

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inSize;
layout(location = 2) in vec2 inCenter;
layout(location = 3) in vec2 inTexCoord;
layout(location = 4) in vec2 inTexExtent;
layout(location = 5) in uvec2 inTexId;

layout(location = 0) out vec2 fragTexCoord;
layout(location = 1) flat out uvec2 fragTexId;
layout(location = 2) out vec4 screenPosAndHeight;


vec2 vertPos[4] = vec2[](
    vec2(0.0, 1.0),
    vec2(0.0, 0.0),
    vec2(1.0, 1.0),
    vec2(1.0, 0.0)
);

out gl_PerVertex
{
	vec4 gl_Position;
};

void main()
{
    vec4 spriteViewCenter = viewUbo.view*vec4(inPos-pc.camPosAndZoom.xyz,1.0);
    spriteViewCenter.z = inPos.z;

    gl_Position = vec4(spriteViewCenter.xyz + vec3((vertPos[gl_VertexIndex] * inSize.xy - inCenter), 0.0), 1.0);

    screenPosAndHeight = vec4(gl_Position.xyz, inSize.z);

    gl_Position.xyz = gl_Position.xyz * vec3(viewUbo.screenSizeFactor, viewUbo.depthOffsetAndFactor.y)
                        + vec3(viewUbo.screenOffset, viewUbo.depthOffsetAndFactor.x);

    fragTexCoord = inTexExtent * vertPos[gl_VertexIndex] + inTexCoord;
    fragTexId    = inTexId;
}


