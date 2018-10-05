#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0, set = 1) uniform sampler samp;
layout(binding = 1, set = 1) uniform texture2DArray textures[128];

layout(location = 0) flat in float fragColorA;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) flat in uvec2 fragAlbedoTexId;
layout(location = 3) flat in uvec2 fragHeightTexId;
layout(location = 4) flat in float depthFactor;

layout(location = 0) out float outAlpha;

void main()
{
    float albedoA = fragColorA * texture(sampler2DArray(textures[fragAlbedoTexId.x], samp), vec3(fragTexCoord,fragAlbedoTexId.y)).a;

    if(albedoA < .05 || albedoA >= .99f)
        discard;

    vec4 heightPixel = texture(sampler2DArray(textures[fragHeightTexId.x], samp), vec3(fragTexCoord,fragHeightTexId.y));

    //if(fragColorA < .99f)
      //  heightPixel.a = 1.0f;

    if(fragColorA > .99f  && heightPixel.a < 1.0f )
        heightPixel.a = 0.0; //discard;

    float height = (heightPixel.r + heightPixel.g + heightPixel.b) * 0.33333333;

    gl_FragDepth = gl_FragCoord.z + height * depthFactor;

    ///I would need to use a different depthBuffer to do that properly
    //outAlpha = heightPixel.a;
    outAlpha = heightPixel.a;

}

