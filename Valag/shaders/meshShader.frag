#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0, set = 0) uniform ViewUBO {
    mat4 view;
    mat4 viewInv;
    vec2 screenOffset;
    vec2 screenSizeFactor;
    vec2 depthOffsetAndFactor;
} viewUbo;
layout(binding = 0, set = 1) uniform sampler samp;
layout(binding = 1, set = 1) uniform texture2DArray textures[128];

layout(location = 0)      in vec4  fragColor;
layout(location = 1)      in vec3  fragRmt;
layout(location = 2)      in vec2  fragUV;
layout(location = 3)      in mat3  fragTBN; //Use 3
// 4
// 5
layout(location = 6) flat in uvec2 fragAlbedoTexId;
layout(location = 7) flat in uvec2 fragHeightTexId;
layout(location = 8) flat in uvec2 fragNormalTexId;
layout(location = 9) flat in uvec2 fragRmtTexId;
layout(location = 10)      in vec4  fragWorldPos;

layout(location = 0) out vec4 outAlbedo;
layout(location = 1) out vec4 outPosition;
layout(location = 2) out vec4 outNormal;
layout(location = 3) out vec4 outRmt;

void main()
{
    outAlbedo = fragColor * texture(sampler2DArray(textures[fragAlbedoTexId.x], samp),
                                    vec3(fragUV,fragAlbedoTexId.y));

    if(outAlbedo.a < .99f)
        discard;

    gl_FragDepth = viewUbo.depthOffsetAndFactor.x + fragWorldPos.z*viewUbo.depthOffsetAndFactor.y;

    outPosition     = vec4(fragWorldPos.xyz, 1.0);

    //I could add GS to improve quality of normal mapping

    //I need to compute normal in tangent space

    vec3 normal = texture(sampler2DArray(textures[fragNormalTexId.x], samp), vec3(fragUV,fragNormalTexId.y)).xyz;
    normal = 2.0*normal - vec3(1.0);
    normal = fragTBN*normal;

    outNormal = vec4(normal,1.0);
    outRmt    = vec4(texture(sampler2DArray(textures[fragRmtTexId.x], samp),
                             vec3(fragUV,fragRmtTexId.y)).xyz  * fragRmt, 1.0);

}

