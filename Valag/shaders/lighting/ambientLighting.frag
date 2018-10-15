#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (constant_id = 0) const int envMapMipsCount = 1;

layout (set = 0, binding = 0) uniform sampler2D samplerAlbedo;
layout (set = 0, binding = 1) uniform sampler2D samplerPosition;
layout (set = 0, binding = 2) uniform sampler2D samplerNormal;
layout (set = 0, binding = 3) uniform sampler2D samplerRmt;

layout (set = 0, binding = 4) uniform sampler2D samplerAlphaAlbedo;
layout (set = 0, binding = 5) uniform sampler2D samplerAlphaPosition;
layout (set = 0, binding = 6) uniform sampler2D samplerAlphaNormal;
layout (set = 0, binding = 7) uniform sampler2D samplerAlphaRmt;

layout (set = 0, binding = 8) uniform sampler2D samplerBentNormals;

layout (set = 0, binding = 9) uniform texture2D brdflut;

/*layout(set = 1, binding = 0) uniform sampler samp;
layout(set = 1, binding = 1) uniform texture2DArray textures[128];*/

layout(set = 1, binding = 0) uniform sampler    samp;
layout(set = 1, binding = 1) uniform texture2D  texEnvMap;
layout(set = 1, binding = 2) uniform AmbientLightingUbo {
    vec4 ambientLight;
    bool enableEnvMap;
} ubo;

layout(push_constant) uniform PER_OBJECT
{
    vec4 camPosAndZoom;
}pc;

layout (location = 0) in vec2 inUV;

layout (location = 0) out vec4 outColor;
layout (location = 1) out vec4 outAlphaColor;

vec2 sampleSphericalMap(vec3 v)
{
    vec2 uv = vec2(atan(v.x, v.y), -asin(v.z));
    uv *= vec2(0.15915494309, 0.31830988618);
    uv += 0.5;
    return uv;
}

//This should be replace by something better
vec3 hash(vec3 a)
{
   a = fract(a*0.8);
   a += dot(a, a.xyz + 19.19);
   return fract((a.xxy + a .yxx) * a.zyx);
}


vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(1.0 - cosTheta, 5.0);
}

vec4 computeAmbientLighting(vec4 fragAlbedo, vec3 fragPos, vec4 fragNormal, vec4 fragBentNormal, vec3 fragRmt)
{

    vec3 ambientLighting = ubo.ambientLight.rgb * ubo.ambientLight.a;

    //Change this for better random (use pregenerated)
    vec3 rVec = hash(fragPos);
    vec3 viewDirection = normalize(pc.camPosAndZoom.xyz - fragPos);
    float NdotV = max(dot(fragNormal.xyz, viewDirection), 0.0);

    //vec3 ortho_viewDirection = vec3(<<cos(45*PI/180)*cos(30*PI/180)<<,
    //                                <<sin(45*PI/180)*cos(30*PI/180)<<,
    //                                <<sin(30*PI/180)<<);

    vec3 surfaceReflection0 = vec3(0.04);
    surfaceReflection0 = mix(surfaceReflection0, fragAlbedo.rgb, fragRmt.g);

    vec3 F = fresnelSchlickRoughness(NdotV, surfaceReflection0, fragRmt.r);
    vec3 kS = F;
    vec3 kD = (1.0 - F)*(1.0 - fragRmt.g);
    //kD *= pow(fragBentNormal.a,2.0);
    vec3 irradiance = ambientLighting;
    float occlusion = max(min(pow(fragBentNormal.a,2.0), pow(1.0-abs(fragBentNormal.z),2.0)), fragNormal.w); //We dont want to occlude truly transparent fragments

    vec3 reflectionView = reflect(-/*ortho_viewDirection*/viewDirection, fragNormal.xyz);
    //reflectionView += mix(vec3(0.0),rVec,fragRmt.r*.25);
    vec3 reflectionColor = ambientLighting;
    //if(enable_map_environmental == true)
    //    reflectionColor = texture2DLod(map_environmental, uv, fragRmt.r*8.0).rgb;
    vec2 envUV = vec2(0,0);
    /*if(!(ubo.envMap.x == 0 && ubo.envMap.y == 0))
    {
        envUV = sampleSphericalMap(normalize(reflectionView));
        reflectionColor = texture(sampler2DArray(textures[ubo.envMap.x], samp),vec3(envUV,ubo.envMap.y)).rgb;
    }*/

    if(ubo.enableEnvMap == true)
    {
        envUV = sampleSphericalMap(normalize(reflectionView));
        reflectionColor = textureLod(sampler2D(texEnvMap,samp), vec2(envUV), fragRmt.r * envMapMipsCount).rgb;
    }

    vec2 envBRDF  = texture(sampler2D(brdflut,samp), vec2(NdotV, fragRmt.r)).rg;
    vec3 specular = reflectionColor  * (F * envBRDF.x + envBRDF.y);

    return vec4(fragAlbedo.rgb * irradiance * occlusion * kD + specular, fragAlbedo.a);
}

void main()
{
    vec4 fragAlbedo = texture(samplerAlbedo, gl_FragCoord.xy);
    vec3 fragPos    = texture(samplerPosition, gl_FragCoord.xy).xyz;
    vec4 fragNormal = texture(samplerNormal, gl_FragCoord.xy);
    fragNormal.xyz  = normalize(fragNormal.xyz);//This should be already normalized heh
    vec3 fragRmt    = texture(samplerRmt, gl_FragCoord.xy).xyz;
    vec4 fragBentNormal = texture(samplerBentNormals, gl_FragCoord.xy);

    outColor = computeAmbientLighting(fragAlbedo, fragPos, fragNormal, fragBentNormal, fragRmt);
	outColor.rgb = pow(outColor.rgb, vec3(2.2));


    fragAlbedo = texture(samplerAlphaAlbedo, gl_FragCoord.xy);
    fragPos    = texture(samplerAlphaPosition, gl_FragCoord.xy).xyz;
    fragNormal = texture(samplerAlphaNormal, gl_FragCoord.xy);
    fragNormal.xyz  = normalize(fragNormal.xyz);//This should be already normalized heh
    fragRmt    = texture(samplerAlphaRmt, gl_FragCoord.xy).xyz;

    outAlphaColor = computeAmbientLighting(fragAlbedo, fragPos, fragNormal, fragBentNormal, fragRmt);
	outAlphaColor.rgb = pow(outAlphaColor.rgb, vec3(2.2));
}

