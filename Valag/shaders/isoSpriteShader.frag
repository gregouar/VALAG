#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0, set = 1) uniform sampler samp;
layout(binding = 1, set = 1) uniform texture2DArray textures[128];

layout(location = 0) in vec4 fragColor;
layout(location = 1) in vec4 fragRmt;
layout(location = 2) in vec2 fragTexCoord;
layout(location = 3) in vec2 fragAlbedoTexId;
layout(location = 4) in vec2 fragHeightTexId;
layout(location = 5) in vec2 fragNormalTexId;
layout(location = 6) in vec2 fragRmtTexId;
layout(location = 7) in float heightFactor;

layout(location = 0) out vec4 outAlbedo;
//layout(location = 1) out float outHeight;
layout(location = 1) out vec4 outNormal;
layout(location = 2) out vec4 outRmt;

void main()
{
    outAlbedo = fragColor * texture(sampler2DArray(textures[int(fragAlbedoTexId.x)], samp), vec3(fragTexCoord,fragAlbedoTexId.y));
    if(outAlbedo.a != 1.0f)
        discard;

    vec3 height = texture(sampler2DArray(textures[int(fragHeightTexId.x)], samp), vec3(fragTexCoord,fragHeightTexId.y)).xyz;
    //outHeight = (height.x + height.y + height.z);
    outNormal = texture(sampler2DArray(textures[int(fragNormalTexId.x)], samp), vec3(fragTexCoord,fragNormalTexId.y));
    outRmt    = texture(sampler2DArray(textures[int(fragRmtTexId.x)], samp), vec3(fragTexCoord,fragRmtTexId.y))  * fragRmt;

    gl_FragDepth = gl_FragCoord.z + ( (height.x + height.y + height.z) * heightFactor)* 0.33333333;
}

