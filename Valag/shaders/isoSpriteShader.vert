#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0, set = 0) uniform ViewUBO {
    mat4 view;
    mat4 viewInv;
    vec2 screenOffset;
    vec2 screenSizeFactor;
    vec2 depthOffsetAndFactor;
} viewUbo;

layout(location = 0) in vec3 inPos;
layout(location = 1) in float inRotation;
layout(location = 2) in vec3 inSize;
layout(location = 3) in vec2 inCenter;
layout(location = 4) in vec4 inColor;
layout(location = 5) in vec3 inRmt;
layout(location = 6) in vec2 inTexCoord;
layout(location = 7) in vec2 inTexExtent;
layout(location = 8) in uvec2 inAlbedoTexId;
layout(location = 9) in uvec2 inHeightTexId;
layout(location = 10) in uvec2 inNormalTexId;
layout(location = 11) in uvec2 inRmtTexId;

layout(location = 0) flat out vec4 fragColor;
layout(location = 1) flat out vec3 fragRmt;
layout(location = 2) out vec2 fragTexCoord;
layout(location = 3) flat out uvec2 fragAlbedoTexId;
layout(location = 4) flat out uvec2 fragHeightTexId;
layout(location = 5) flat out uvec2 fragNormalTexId;
layout(location = 6) flat out uvec2 fragRmtTexId;
layout(location = 7) flat out float depthFactor;
layout(location = 8) out vec4 screenPosAndHeight;


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
    vec4 spriteViewCenter = viewUbo.view*vec4(inPos,1.0);
    spriteViewCenter = vec4(spriteViewCenter.xyz/spriteViewCenter.w,1.0);
    spriteViewCenter.z = inPos.z;

    float c = cos(inRotation);
    float s = sin(inRotation);

    gl_Position = vec4(spriteViewCenter.xyz + vec3( mat2(c,s,-s,c) * (vertPos[gl_VertexIndex] * inSize.xy - inCenter), 0.0), 1.0);

    screenPosAndHeight = vec4(gl_Position.xyz, inSize.z);

    gl_Position.xyz = gl_Position.xyz * vec3(viewUbo.screenSizeFactor, viewUbo.depthOffsetAndFactor.y)
                        + vec3(viewUbo.screenOffset, viewUbo.depthOffsetAndFactor.x);

    fragTexCoord = inTexExtent * vertPos[gl_VertexIndex] + inTexCoord;
    fragColor    = inColor;
    fragRmt      = inRmt;
    fragAlbedoTexId    = inAlbedoTexId;
    fragHeightTexId    = inHeightTexId;
    fragNormalTexId    = inNormalTexId;
    fragRmtTexId       = inRmtTexId;
    depthFactor        = inSize.z * viewUbo.depthOffsetAndFactor.y;
}


