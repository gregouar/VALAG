#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (set = 0, binding = 0) uniform sampler2D samplerAlbedo;
layout (set = 0, binding = 1) uniform sampler2D samplerPosition;
layout (set = 0, binding = 2) uniform sampler2D samplerNormal;
layout (set = 0, binding = 3) uniform sampler2D samplerRmt;

layout (set = 0, binding = 4) uniform sampler2D samplerAlphaAlbedo;
layout (set = 0, binding = 5) uniform sampler2D samplerAlphaPosition;
layout (set = 0, binding = 6) uniform sampler2D samplerAlphaNormal;
layout (set = 0, binding = 7) uniform sampler2D samplerAlphaRmt;

layout(set = 0, binding = 8) uniform AmbientLightingUbo {
    vec4 viewPos;
    vec4 ambientLight;
} ubo;
layout (set = 0, binding = 9) uniform sampler2D samplerBrdflut;

layout (location = 0) in vec2 inUV;

layout (location = 0) out vec4 outColor;
layout (location = 1) out vec4 outAlphaColor;


vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(1.0 - cosTheta, 5.0);
}

vec4 computeAmbientLighting(vec4 fragAlbedo, vec3 fragPos, vec3 fragNormal, vec3 fragRmt)
{

    vec3 ambientLighting = ubo.ambientLight.rgb * ubo.ambientLight.a;

    //vec3 rVec = Hash(fragPos);
    //vec2 rVec2 = rVec.xy;
    vec3 viewDirection = normalize(ubo.viewPos.xyz - fragPos);

    //vec3 ortho_viewDirection = vec3(<<cos(45*PI/180)*cos(30*PI/180)<<,
    //                                <<sin(45*PI/180)*cos(30*PI/180)<<,
    //                                <<sin(30*PI/180)<<);

    vec3 surfaceReflection0 = vec3(0.04);
    surfaceReflection0 = mix(surfaceReflection0, fragAlbedo.rgb, fragRmt.g);
    vec3 FAmbient = fresnelSchlickRoughness(max(dot(fragNormal, viewDirection), 0.0), surfaceReflection0, fragRmt.r);
    vec3 kSAmbient = FAmbient;
    vec3 kDAmbient = (1.0 - kSAmbient)*(1.0 - fragRmt.g);
    vec3 irradianceAmbient = ambientLighting;

    //vec3 reflectionView = reflect(-ortho_viewDirection, direction);
    //reflectionView += mix(vec3(0.0),rVec,materialPixel.r*.25);
    //vec2 uv = SampleSphericalMap(normalize(reflectionView));
    vec3 reflectionColor = ambientLighting;
    //if(enable_map_environmental == true)
    //    reflectionColor = texture2DLod(map_environmental, uv, materialPixel.r*8.0).rgb;

    vec2 envBRDF  = texture(samplerBrdflut, vec2(max(dot(fragNormal, viewDirection), 0.0), /*1.0-*/fragRmt.r)).rg;
    vec3 specularAmbient = reflectionColor* (FAmbient * envBRDF.x + envBRDF.y);

    return vec4(fragAlbedo.rgb * irradianceAmbient + specularAmbient, fragAlbedo.a);
}

void main()
{
    vec4 fragAlbedo = texture(samplerAlbedo, gl_FragCoord.xy);
    vec3 fragPos    = texture(samplerPosition, gl_FragCoord.xy).xyz;
    vec3 fragNormal = normalize(texture(samplerNormal, gl_FragCoord.xy).xyz);
    vec3 fragRmt    = texture(samplerRmt, gl_FragCoord.xy).xyz;
    outColor = computeAmbientLighting(fragAlbedo, fragPos, fragNormal, fragRmt);


    fragAlbedo = texture(samplerAlphaAlbedo, gl_FragCoord.xy);
    fragPos    = texture(samplerAlphaPosition, gl_FragCoord.xy).xyz;
    fragNormal = normalize(texture(samplerAlphaNormal, gl_FragCoord.xy).xyz);
    fragRmt    = texture(samplerAlphaRmt, gl_FragCoord.xy).xyz;

    outAlphaColor = computeAmbientLighting(fragAlbedo, fragPos, fragNormal, fragRmt);
}

