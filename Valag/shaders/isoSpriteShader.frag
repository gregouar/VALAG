#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0, set = 1) uniform sampler samp;
layout(binding = 1, set = 1) uniform texture2DArray textures[128];

layout(location = 0) flat in vec4 fragColor;
layout(location = 1) flat in vec3 fragRmt;
layout(location = 2) in vec2 fragTexCoord;
layout(location = 3) flat in uvec2 fragAlbedoTexId;
layout(location = 4) flat in uvec2 fragHeightTexId;
layout(location = 5) flat in uvec2 fragNormalTexId;
layout(location = 6) flat in uvec2 fragRmtTexId;
layout(location = 7) flat in float heightFactor;

layout(location = 0) out vec4 outAlbedo;
//layout(location = 1) out float outHeight;
layout(location = 1) out vec4 outNormal;
layout(location = 2) out vec4 outRmt;

void main()
{
    outAlbedo = fragColor * texture(sampler2DArray(textures[fragAlbedoTexId.x], samp), vec3(fragTexCoord,fragAlbedoTexId.y));

    if(outAlbedo.a < .99f)
        discard;

    vec3 height = texture(sampler2DArray(textures[fragHeightTexId.x], samp), vec3(fragTexCoord,fragHeightTexId.y)).xyz;
    //outHeight = (height.x + height.y + height.z);
    outNormal = texture(sampler2DArray(textures[fragNormalTexId.x], samp), vec3(fragTexCoord,fragNormalTexId.y));
    outRmt    = vec4(texture(sampler2DArray(textures[fragRmtTexId.x], samp), vec3(fragTexCoord,fragRmtTexId.y)).xyz  * fragRmt, 1.0);

    gl_FragDepth = gl_FragCoord.z + ( (height.x + height.y + height.z) * heightFactor)* 0.33333333;

    //if(outAlbedo.a < .98f)
        //gl_FragDepth = 0.000000001;
        //discard;
}

