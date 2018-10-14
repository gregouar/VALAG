#version 450
#extension GL_ARB_separate_shader_objects : enable

//1.0/pi ~ 0.31830988618

layout(early_fragment_tests) in;

layout (constant_id = 0) const float const_aoIntensity = 1;
layout (constant_id = 1) const float const_gioIntensity = 1;

layout (set = 1, binding = 0) uniform sampler2D samplerAlbedo;
layout (set = 1, binding = 1) uniform sampler2D samplerPosition;
layout (set = 1, binding = 2) uniform sampler2D samplerNormal;
layout (set = 1, binding = 3) uniform sampler2D samplerRmt;
layout (set = 1, binding = 4) uniform sampler2D samplerBentNormal;

/*layout (set = 1, binding = 4) uniform sampler2D samplerAlphaAlbedo;
layout (set = 1, binding = 5) uniform sampler2D samplerAlphaPosition;
layout (set = 1, binding = 6) uniform sampler2D samplerAlphaNormal;
layout (set = 1, binding = 7) uniform sampler2D samplerAlphaRmt;*/

/*layout(set = 1, binding = 5) uniform AmbientLightingUbo {
    //vec4 viewPos;
    vec4 ambientLight;
    bool enableEnvMap;
    //uvec2 envMap;
} ubo;*/

layout(push_constant) uniform PER_OBJECT
{
    vec4 camPosAndZoom;
}pc;


//layout(binding = 0, set = 1) uniform sampler samp;
//layout(binding = 1, set = 1) uniform texture2DArray textures[128];

layout(location = 0) flat in vec4  lightPos;
layout(location = 1) flat in vec3  lightColor;
layout(location = 2) flat in float lightRadiusInv;

layout(location = 0) out vec4 outColor;
//layout(location = 1) out vec4 outColorAlpha;

vec3 FresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a         = roughness*roughness;
    float a2        = a*a;
    float NdotH     = max(dot(N, H), 0.0);
    float NdotH2    = NdotH*NdotH;
    float nom       = a2;
    float denom     = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = 3.1415926535 * denom * denom;

    return nom / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) * 0.125;
    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2  = GeometrySchlickGGX(NdotV, roughness);
    float ggx1  = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

vec4 ComputeLighting(vec4 fragAlbedo, vec3 fragPos, vec4 fragNormal, vec4 fragBentNormal, vec3 fragRmt)
{
    vec4 lighting = vec4(0.0);

    //if(fragAlbedo.a < .1)
      //  return lighting;

    float attenuation = 0.0;
    vec3 lightDirection = vec3(0.0);

    vec3 surfaceReflection0 = vec3(0.04);
    surfaceReflection0 = mix(surfaceReflection0, fragAlbedo.rgb, fragRmt.g);

    vec3 viewDirection = normalize(pc.camPosAndZoom.xyz - fragPos);

    if(lightPos.w == 0.0)
    {
        lightDirection = -lightPos.xyz;
        attenuation = 1.0;
    }
    else
    {
        lightDirection = lightPos.xyz - fragPos.xyz;
        float dist = length(lightDirection)*0.01;
        float dr = dist*lightRadiusInv;
        float sqrtnom = 1.0 - dr*dr*dr*dr;
        if(sqrtnom >= 0.0)
            attenuation = clamp(sqrtnom*sqrtnom/(dist*dist+1.0),0.0,1.0);
    }

    if(attenuation > 0.0)
    {

        //Compute attenuation from shadows here

        float gio = abs(fragBentNormal.z);
        fragBentNormal.xyz = vec3(fragBentNormal.xy, /*sign(fragBentNormal.z) * */ sqrt(1.0 - fragBentNormal.x*fragBentNormal.x - fragBentNormal.y*fragBentNormal.y));

        lightDirection      = normalize(lightDirection);
        vec3 halfwayVector  = normalize(viewDirection + lightDirection);
        float NdotL         = max(dot(fragNormal.xyz, lightDirection), 0.0);
        float BNdotL        = clamp(dot(fragBentNormal.xyz, lightDirection)/*+0.25*/, 0.0, 1.0);
        //float BNdotL        = min(dot(fragBentNormal.xyz, lightDirection), 0.0);

        vec3 radiance   = attenuation*lightColor.rgb;
        float NDF   = DistributionGGX(fragNormal.xyz, halfwayVector, fragRmt.r);
        float G     = GeometrySmith(fragNormal.xyz, viewDirection, lightDirection, fragRmt.r);
        vec3 F      = FresnelSchlick(max(dot(halfwayVector, viewDirection), 0.0), surfaceReflection0);
        vec3 kS     = F;
        vec3 kD     = vec3(1.0) - kS;
        kD         *= 1.0 - fragRmt.g;


        float ao  = fragBentNormal.a;
        ao = pow(ao, const_aoIntensity);

        gio =  1.0 - max((1.0 - BNdotL) * gio, (gio-0.5));
        gio = pow(gio, const_gioIntensity);

        float occlusion = max(min(ao, gio),fragNormal.w);  //We dont want to occlude truly transparent fragments


        vec3 nominator      = NDF * G * F;
        float denominator   = 4.0 * max(dot(fragNormal.xyz, viewDirection), 0.0) * NdotL;
        vec3 specular       = nominator / max(denominator, 0.01);
        lighting.rgb       += (kD * fragAlbedo.rgb * 0.31830988618 + specular)  * NdotL * radiance * occlusion;

        //Translucency
        float t         = fragRmt.b;
        lighting.rgb   -= (kD * fragAlbedo.rgb*0.31830988618) * radiance * min(dot(fragNormal.xyz, lightDirection), 0.0)*t* occlusion;

    }

    return lighting;
}

void main()
{
    vec4 fragAlbedo = texture(samplerAlbedo, gl_FragCoord.xy);
    vec3 fragPos    = texture(samplerPosition, gl_FragCoord.xy).xyz;
    vec4 fragNormal = texture(samplerNormal, gl_FragCoord.xy);
    fragNormal.xyz  = normalize(fragNormal.xyz);//This should be already normalized heh
    vec3 fragRmt    = texture(samplerRmt, gl_FragCoord.xy).xyz;
    vec4 fragBentNormal = texture(samplerBentNormal, gl_FragCoord.xy);

	fragAlbedo.rgb = pow(fragAlbedo.rgb, vec3(2.2));

    outColor = ComputeLighting(fragAlbedo, fragPos, fragNormal, fragBentNormal, fragRmt);//vec4(lightColor.rgb,0.0);
	//outColor.g = 1.0;

    /*fragAlbedo = texture(samplerAlphaAlbedo, gl_FragCoord.xy);
    fragPos    = texture(samplerAlphaPosition, gl_FragCoord.xy).xyz;
    fragNormal = normalize(texture(samplerAlphaNormal, gl_FragCoord.xy).xyz);
    fragRmt    = texture(samplerAlphaRmt, gl_FragCoord.xy).xyz;

    outColorAlpha = ComputeLighting(fragAlbedo, fragPos, fragNormal, fragRmt);
	outColorAlpha.rgb = pow(outColorAlpha.rgb, vec3(2.2));*/
}

