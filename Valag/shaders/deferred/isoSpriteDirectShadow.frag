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

layout(push_constant) uniform PER_OBJECT
{
    vec4 camPosAndZoom;
}pc;

layout(location = 0) flat in vec4 fragColor;
layout(location = 1) flat in vec3 fragRmt;
layout(location = 2)      in vec2 fragTexCoord;
layout(location = 3) flat in uvec2 fragAlbedoTexId;
layout(location = 4) flat in uvec2 fragHeightTexId;
layout(location = 5) flat in uvec2 fragNormalTexId;
layout(location = 6) flat in uvec2 fragRmtTexId;
//layout(location = 7) flat in float depthFactor;
layout(location = 7) in vec4 screenPosAndHeight;

layout(location = 0) out vec4 outAlbedo;
layout(location = 1) out vec4 outPosition;
layout(location = 2) out vec4 outNormal;
layout(location = 3) out vec4 outRmt;

void main()
{
    outAlbedo = fragColor * texture(sampler2DArray(textures[fragAlbedoTexId.x], samp),
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
        discard;

    /*outAlbedo = fragColor * texture(sampler2DArray(textures[fragAlbedoTexId.x], samp),
                                    vec3(fragTexCoord,fragAlbedoTexId.y));

    if(outAlbedo.a < .99f)
        discard;

    vec4 heightPixel = texture(sampler2DArray(textures[fragHeightTexId.x], samp),
                               vec3(fragTexCoord,fragHeightTexId.y));

    float height = (heightPixel.r + heightPixel.g + heightPixel.b) * 0.33333333;
    float fragHeight = screenPosAndHeight.z + height * screenPosAndHeight.w;

    vec2 fragWorldPos = screenPosAndHeight.xy;
    fragWorldPos.y -= (fragHeight - viewUbo.viewInv[3][2]) * viewUbo.view[2][1];
    fragWorldPos = vec4(viewUbo.viewInv*vec4(fragWorldPos.xy,0.0,1.0)).xy;

    gl_FragDepth = viewUbo.depthOffsetAndFactor.x + fragHeight * viewUbo.depthOffsetAndFactor.y;

    vec3 normal = vec3(0.5,0.5,1.0);
    if(!(fragNormalTexId.x == 0 && fragNormalTexId.y == 0))
        normal = texture(sampler2DArray(textures[fragNormalTexId.x], samp), vec3(fragTexCoord,fragNormalTexId.y)).xyz;
    normal = 2.0*normal - vec3(1.0);
    normal = vec4(vec4(normal,1.0)*viewUbo.view).xyz;


    outPosition     = vec4(fragWorldPos.xy, fragHeight, 1.0);
    outNormal       = vec4(normal,1.0);
    outRmt          = vec4(texture(sampler2DArray(textures[fragRmtTexId.x], samp), vec3(fragTexCoord,fragRmtTexId.y)).xyz  * fragRmt, 1.0);*/
}

