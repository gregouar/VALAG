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


layout(location = 0) flat   in vec3 viewLightDirection;
layout(location = 1)        in vec2 inUV;
layout(location = 2)        in vec2 spriteTexCoord;
layout(location = 3)        in vec2 spriteTexExtent;
layout(location = 4) flat   in uvec2 spriteAlbedoTexId;
layout(location = 5) flat   in uvec2 spriteHeightTexId;
layout(location = 6)        in vec2 spritePos;
layout(location = 7)        in vec3 spriteSize;

layout(location = 0) out vec4 outShadow;



void main()
{
    outShadow = texture(sampler2DArray(textures[spriteHeightTexId.x], samp),
                                    vec3(inUV,spriteHeightTexId.y));

    /*outAlbedo = fragColor * texture(sampler2DArray(textures[fragAlbedoTexId.x], samp),
                                    vec3(fragTexCoord,fragAlbedoTexId.y));


    vec4 heightPixel = texture(sampler2DArray(textures[fragHeightTexId.x], samp),
                               vec3(fragTexCoord,fragHeightTexId.y));

    float height = (heightPixel.r + heightPixel.g + heightPixel.b) * 0.33333333;
    float fragHeight = screenPosAndHeight.z + height * screenPosAndHeight.w;

    vec2 fragWorldPos = screenPosAndHeight.xy;
    fragWorldPos.y -= (fragHeight - pc.camPosAndZoom.z) * viewUbo.view[2][1];
    fragWorldPos = vec4(viewUbo.viewInv*vec4(fragWorldPos.xy,0.0,1.0)).xy;
    fragWorldPos += pc.camPosAndZoom.xy;

    gl_FragDepth = viewUbo.depthOffsetAndFactor.x + fragHeight * viewUbo.depthOffsetAndFactor.y;

    outPosition = vec4(fragWorldPos.xy, fragHeight, 0.0);

    vec3 normal = vec3(0.5,0.5,1.0);
    if(!(fragNormalTexId.x == 0 && fragNormalTexId.y == 0))
        normal = texture(sampler2DArray(textures[fragNormalTexId.x], samp), vec3(fragTexCoord,fragNormalTexId.y)).xyz;
    normal = 2.0*normal - vec3(1.0);

    normal = vec4(vec4(normal,0.0)*viewUbo.view).xyz;
    outNormal = vec4(normal,0.0);

    outRmt = vec4(texture(sampler2DArray(textures[fragRmtTexId.x], samp), vec3(fragTexCoord,fragRmtTexId.y)).xyz  * fragRmt, 1.0);

    if(outAlbedo.a < .99f)
        discard;*/
}

