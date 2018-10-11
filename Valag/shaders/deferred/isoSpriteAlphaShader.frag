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

//layout(set = 2, binding = 0) uniform sampler2D samplerTransparent;
layout(set = 2, binding = 0) uniform sampler2D samplerOpacPosition;

layout(location = 0) flat in vec4 fragColor;
layout(location = 1) flat in vec3 fragRmt;
layout(location = 2) in vec2 fragTexCoord;
layout(location = 3) flat in uvec2 fragAlbedoTexId;
layout(location = 4) flat in uvec2 fragHeightTexId;
layout(location = 5) flat in uvec2 fragNormalTexId;
layout(location = 6) flat in uvec2 fragRmtTexId;
layout(location = 7) in vec4 screenPosAndHeight;

layout(location = 0) out vec4 outAlbedo;
layout(location = 1) out vec4 outPosition;
layout(location = 2) out vec4 outNormal;
layout(location = 3) out vec4 outRmt;

void main()
{
    outAlbedo = fragColor * texture(sampler2DArray(textures[fragAlbedoTexId.x], samp),
                                    vec3(fragTexCoord,fragAlbedoTexId.y));

    //if(outAlbedo.a < .05 || outAlbedo.a >= .99f)
      //  discard;

    vec4 heightPixel = texture(sampler2DArray(textures[fragHeightTexId.x], samp),
                               vec3(fragTexCoord,fragHeightTexId.y));

    float height        = (heightPixel.r + heightPixel.g + heightPixel.b) * 0.33333333;
    float fragHeight    = screenPosAndHeight.z + height * screenPosAndHeight.w;

    vec2 fragWorldPos = screenPosAndHeight.xy;
    fragWorldPos.y -= (fragHeight - viewUbo.viewInv[3][2]) * viewUbo.view[2][1];
    fragWorldPos = vec4(viewUbo.viewInv*vec4(fragWorldPos.xy,0.0,1.0)).xy;

    if(outAlbedo.a < .05)
        discard;

    vec2 samplerOpac = texture(samplerOpacPosition, gl_FragCoord.xy).zw;

    if(outAlbedo.a >= .99f)
    {
        //float opacHeight = texture(samplerOpacPosition, gl_FragCoord.xy).z;
        float depth = (samplerOpac.x-fragHeight)/2.0;
        if(depth > 1.0 || depth < 0.00000001)
            discard;
        outAlbedo.a = mix(0.5,0.0,depth);
        fragHeight = samplerOpac.x;
    }

    ///Replace this by comparaison of alpha (only possible if working with a different depthBuffer (that would contain the opacity
    //uint existingTrulyTransparentPixel = uint(texture(samplerTransparent, gl_FragCoord.xy /* *viewUbo.screenSizeFactor.xy*0.5 */).a);
    uint existingTrulyTransparentPixel = uint(samplerOpac.y);
    //float existingTrulyTransparentPixel = texture(samplerTransparent, gl_FragCoord.xy).r;

    //if(fragColor.a < .99f)
      //  heightPixel.a = 1.0f;

    if(existingTrulyTransparentPixel != 0 && fragColor.a > .99f && heightPixel.a < 1.0f)
    //if(heightPixel.a < existingTrulyTransparentPixel)
        discard;

    outPosition         = vec4(fragWorldPos.xy, fragHeight, outAlbedo.a);

    vec3 normal = vec3(0.5,0.5,1.0);
    if(!(fragNormalTexId.x == 0 && fragNormalTexId.y == 0))
        normal = texture(sampler2DArray(textures[fragNormalTexId.x], samp),
                          vec3(fragTexCoord,fragNormalTexId.y)).xyz;
    normal = 2.0*normal - vec3(1.0);
    normal = vec4(vec4(normal,1.0)*viewUbo.view).xyz;

    gl_FragDepth = viewUbo.depthOffsetAndFactor.x + fragHeight * viewUbo.depthOffsetAndFactor.y;

    outNormal       = vec4(normal,samplerOpac.y);
    outRmt          = vec4(texture(sampler2DArray(textures[fragRmtTexId.x], samp),
                                   vec3(fragTexCoord,fragRmtTexId.y)).xyz  * fragRmt, 1.0);
}

