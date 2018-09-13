#version 450
#extension GL_ARB_separate_shader_objects : enable


layout(binding = 0, set = 1) uniform sampler samp;
layout(binding = 1, set = 1) uniform texture2DArray textures[128];

layout(location = 0) in vec4 fragColor;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec2 fragTexId;

layout(location = 0) out vec4 outColor;

void main()
{
    outColor = texture(sampler2DArray(textures[int(fragTexId.x)], samp), vec3(fragTexCoord,fragTexId.y))  * fragColor ;
    if(outColor.a == 0)
        discard;
}

