#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0, set = 0) uniform ViewUBO {
    mat4 view;
    mat4 viewInv;
    vec2 screenOffset;
    vec2 screenSizeFactor;
    vec2 depthOffsetAndFactor;
} viewUbo;

//Mesh vertex
layout(location = 0) in vec3  inPos;
layout(location = 1) in vec2  inUV;
layout(location = 2) in vec3  inNormal;
/*layout(location = 3) in vec4  inColor;
layout(location = 4) in vec3  inRmt;
layout(location = 5) in uvec2 inAlbedoTexId;
layout(location = 6) in uvec2 inHeightTexId;
layout(location = 7) in uvec2 inNormalTexId;
layout(location = 8) in uvec2 inRmtTexId;*/

//Instance vertex
layout(location = 3)  in mat4  inModel;
layout(location = 7) in vec4  inInstanceColor;
layout(location = 8) in vec3  inInstanceRmt;
layout(location = 9) in uvec2 inAlbedoTexId;
layout(location = 10) in uvec2 inHeightTexId;
layout(location = 11) in uvec2 inNormalTexId;
layout(location = 12) in uvec2 inRmtTexId;

//Out
layout(location = 0)      out vec4 fragColor;
layout(location = 1)      out vec3 fragRmt;
layout(location = 2)      out vec2 fragUV;
layout(location = 3)      out vec3 fragNormal;
layout(location = 4) flat out uvec2 fragAlbedoTexId;
layout(location = 5) flat out uvec2 fragHeightTexId;
layout(location = 6) flat out uvec2 fragNormalTexId;
layout(location = 7) flat out uvec2 fragRmtTexId;
layout(location = 8)      out vec4  fragWorldPos;

out gl_PerVertex {
    vec4 gl_Position;
};

void main()
{
    fragWorldPos = inModel*vec4(inPos,1.0);
    fragNormal   = vec4(inModel*vec4(inNormal,1.0)).xyz;

    gl_Position = viewUbo.view*fragWorldPos;
    gl_Position.xyz = gl_Position.xyz * vec3(viewUbo.screenSizeFactor, 0.0)
                        + vec3(viewUbo.screenOffset, 0.0);

    fragWorldPos = vec4(fragWorldPos.xyz/*/fragWorldPos.w*/, 1.0);

    fragUV      = inUV;
    fragColor   = /*inColor **/ inInstanceColor;
    fragRmt     = /*inRmt **/ inInstanceRmt;
    fragAlbedoTexId    = inAlbedoTexId;
    fragHeightTexId    = inHeightTexId;
    fragNormalTexId    = inNormalTexId;
    fragRmtTexId       = inRmtTexId;

}


