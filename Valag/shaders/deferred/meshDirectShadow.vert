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
    vec2 shadowShift;
    vec2 lightXYonZ;
}pc;


//Mesh vertex
layout(location = 0) in vec3  inPos;
layout(location = 1) in vec2  inUV;
layout(location = 2) in vec3  inNormal;
layout(location = 3) in vec3  inTangent;
layout(location = 4) in vec3  inBitangent;

//Instance vertex
layout(location = 5)  in mat4  inModel; //Use 4
//6
//7
//8
layout(location = 9) in vec4  inInstanceColor;
layout(location = 10) in vec3  inInstanceRmt;
layout(location = 11) in uvec2 inAlbedoTexId;
layout(location = 12) in uvec2 inHeightTexId;
layout(location = 13) in uvec2 inNormalTexId;
layout(location = 14) in uvec2 inRmtTexId;
layout(location = 15) in float inTexThickness;

//Out
layout(location = 0)       out vec2 fragUV;
layout(location = 1)  flat out uvec2 fragAlbedoTexId;

out gl_PerVertex {
    vec4 gl_Position;
};

void main()
{
    vec4 worldPos = inModel * vec4(inPos,1.0);

    vec4 projWorldPos = vec4(worldPos.xy - worldPos.z*pc.lightXYonZ,
                             0.0,1.0);

    gl_Position = viewUbo.view*(projWorldPos - vec4(pc.camPosAndZoom.xyz,0.0));
    gl_Position.z = worldPos.z;

    gl_Position.xy -= pc.shadowShift * 0.5;
    vec2 shadowSizeFactor = 2.0/(2.0/viewUbo.screenSizeFactor+abs(pc.shadowShift));

    gl_Position.xyz = gl_Position.xyz * vec3(shadowSizeFactor, viewUbo.depthOffsetAndFactor.y)
                        + vec3(viewUbo.screenOffset, viewUbo.depthOffsetAndFactor.x);

    fragUV          = inUV;
    fragAlbedoTexId = inAlbedoTexId;
}


