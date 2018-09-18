#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0, set = 0) uniform ViewUBO {
    mat4 view;
} viewUbo;

layout(location = 0) in mat4 inModel;
layout(location = 4) in vec4 inColor;
layout(location = 5) in vec4 inRmt;
layout(location = 6) in vec2 inTexCoord;
layout(location = 7) in vec2 inTexExtent;
layout(location = 8) in vec2 inAlbedoTexId;
layout(location = 9) in vec2 inHeightTexId;
layout(location = 10) in vec2 inNormalTexId;
layout(location = 11) in vec2 inRmtTexId;

layout(location = 0) out vec4 fragColor;
layout(location = 1) out vec4 fragRmt;
layout(location = 2) out vec2 fragTexCoord;
layout(location = 3) out vec2 fragAlbedoTexId;
layout(location = 4) out vec2 fragHeightTexId;
layout(location = 5) out vec2 fragNormalTexId;
layout(location = 6) out vec2 fragRmtTexId;


vec2 vertPos[4] = vec2[](
    vec2(0.0, 1.0),
    vec2(0.0, 0.0),
    vec2(1.0, 1.0),
    vec2(1.0, 0.0)
);

void main()
{
    gl_Position =  viewUbo.view * inModel *  vec4(vertPos[gl_VertexIndex], 0.0, 1.0);
    fragTexCoord = inTexExtent * vertPos[gl_VertexIndex] + inTexCoord;
    fragColor    = inColor;
    fragRmt      = inRmt;
    fragAlbedoTexId    = inAlbedoTexId;
    fragHeightTexId    = inHeightTexId;
    fragNormalTexId    = inNormalTexId;
    fragRmtTexId       = inRmtTexId;
}


