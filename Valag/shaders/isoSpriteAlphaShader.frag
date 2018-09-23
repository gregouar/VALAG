#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(set = 0, binding = 0) uniform ViewUBO {
    mat4 view;
    mat4 viewInv;
    vec2 screenOffset;
    vec2 screenSizeFactor;
    vec2 depthOffsetAndFactor;
} viewUbo;
layout(set = 1, binding = 0) uniform sampler samp;
layout(set = 1, binding = 1) uniform texture2DArray textures[128];

layout(set = 2, binding = 0) uniform sampler2D samplerTransparent;

layout(location = 0) flat in vec4 fragColor;
layout(location = 1) flat in vec3 fragRmt;
layout(location = 2) in vec2 fragTexCoord;
layout(location = 3) flat in uvec2 fragAlbedoTexId;
layout(location = 4) flat in uvec2 fragHeightTexId;
layout(location = 5) flat in uvec2 fragNormalTexId;
layout(location = 6) flat in uvec2 fragRmtTexId;
layout(location = 7) flat in float depthFactor;
layout(location = 8) in vec4 screenPosAndHeight;

layout(location = 0) out vec4 outAlbedo;
layout(location = 1) out vec4 outPosition;
layout(location = 2) out vec4 outNormal;
layout(location = 3) out vec4 outRmt;

void main()
{
    outAlbedo = fragColor * texture(sampler2DArray(textures[fragAlbedoTexId.x], samp), vec3(fragTexCoord,fragAlbedoTexId.y));

    if(outAlbedo.a < .05 || outAlbedo.a >= .99f)
        discard;

    vec4 heightPixel = texture(sampler2DArray(textures[fragHeightTexId.x], samp), vec3(fragTexCoord,fragHeightTexId.y));

    uint existingTrulyTransparentPixel = uint(texture(samplerTransparent, gl_FragCoord.xy /* *viewUbo.screenSizeFactor.xy*0.5 */).r);

    if(existingTrulyTransparentPixel != 0 && fragColor.a > .99f && heightPixel.a < 1.0f)
        discard;

    float height = (heightPixel.r + heightPixel.g + heightPixel.b) * 0.33333333;

    gl_FragDepth = gl_FragCoord.z + height * depthFactor;


    vec4 fragWorldPos = vec4(screenPosAndHeight.xyz,1.0);
    fragWorldPos.y -= height * screenPosAndHeight.w * viewUbo.view[2][1];
    fragWorldPos = viewUbo.viewInv * vec4(fragWorldPos.xy,0.0,1.0) +
                    vec4(0.0,0.0, screenPosAndHeight.z + height * screenPosAndHeight.w, 0.0);

    outPosition     = vec4(fragWorldPos.xyz, 0.0);
    outNormal       = vec4(texture(sampler2DArray(textures[fragNormalTexId.x], samp), vec3(fragTexCoord,fragNormalTexId.y)).xyz, 1.0);
    outRmt          = vec4(texture(sampler2DArray(textures[fragRmtTexId.x], samp), vec3(fragTexCoord,fragRmtTexId.y)).xyz  * fragRmt, 1.0);


}

