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


//Mesh vertex
layout(location = 0) in vec3  inPos;
layout(location = 1) in vec2  inUV;
layout(location = 2) in vec3  inNormal;
layout(location = 3) in vec3  inTangent;
layout(location = 4) in vec3  inBitangent;
/*layout(location = 3) in vec4  inColor;
layout(location = 4) in vec3  inRmt;
layout(location = 5) in uvec2 inAlbedoTexId;
layout(location = 6) in uvec2 inHeightTexId;
layout(location = 7) in uvec2 inNormalTexId;
layout(location = 8) in uvec2 inRmtTexId;*/

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
layout(location = 0)      out vec4 fragColor;
layout(location = 1)      out vec3 fragRmt;
layout(location = 2)      out vec2 fragUV;
layout(location = 3)      out mat3 fragTBN; //Use 3
//4
//5
layout(location = 6)  flat out uvec2 fragAlbedoTexId;
layout(location = 7)  flat out uvec2 fragHeightTexId;
layout(location = 8)  flat out uvec2 fragNormalTexId;
layout(location = 9)  flat out uvec2 fragRmtTexId;
layout(location = 10)      out vec4  fragWorldPos;
layout(location = 11) flat out float fragTexThickness;

out gl_PerVertex {
    vec4 gl_Position;
};

void main()
{
    fragWorldPos = inModel * vec4(inPos,1.0);
    //fragNormal   = vec4(inModel*vec4(inNormal,1.0)).xyz;
    vec3 T      = normalize(vec4(inModel * vec4(inTangent,   0.0)).xyz);
    vec3 B      = normalize(vec4(inModel * vec4(inBitangent, 0.0)).xyz);
    vec3 N      = normalize(vec4(inModel * vec4(inNormal,    0.0)).xyz);
    fragTBN     = mat3(T, B, N);

    gl_Position = viewUbo.view*(fragWorldPos - vec4(pc.camPosAndZoom.xyz,0.0));
    gl_Position.xy = gl_Position.xy/gl_Position.w;

    gl_Position.z = fragWorldPos.z;
    gl_Position.xyz = gl_Position.xyz * vec3(viewUbo.screenSizeFactor, viewUbo.depthOffsetAndFactor.y)
                        + vec3(viewUbo.screenOffset, viewUbo.depthOffsetAndFactor.x);

    fragWorldPos = vec4(fragWorldPos.xyz/*/fragWorldPos.w*/, 1.0);

    fragUV      = inUV;
    fragColor   = /*inColor **/ inInstanceColor;
    fragRmt     = /*inRmt **/ inInstanceRmt;
    fragAlbedoTexId    = inAlbedoTexId;
    fragHeightTexId    = inHeightTexId;
    fragNormalTexId    = inNormalTexId;
    fragRmtTexId       = inRmtTexId;
    fragTexThickness   = inTexThickness;

}


