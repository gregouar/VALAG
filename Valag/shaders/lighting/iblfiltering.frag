#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (constant_id = 0) const int mipsCount = 1;

layout(binding = 0, set = 0) uniform sampler samp;
layout(binding = 1, set = 0) uniform texture2DArray textures[128];

layout(push_constant) uniform PER_OBJECT
{
	float   roughness;
	int     srcId;
	int     srcLayer;
}pc;

layout (location = 0) in vec2 inUV;

layout (location = 0) out vec4 outColor;

vec2 sampleSphericalMap(vec3 v)
{
    vec2 uv = vec2(atan(v.x, v.y), -asin(v.z));
    uv *= vec2(0.15915494309, 0.31830988618);
    uv += 0.5;
    return uv;
}

vec3 sphericalToCartesian(vec2 v)
{
    v -= 0.5;
    v *= vec2(2*3.1415926535, 3.1415926535);
    float sinTheta = sin(v.x);
    vec3 n = vec3(sinTheta, sqrt(1-sinTheta*sinTheta), -sin(v.y));
    return n;
}

float radicalInverse_VdC(uint bits)
{
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}

vec2 hammersley(uint i, uint N)
{
    return vec2(float(i)/float(N), radicalInverse_VdC(i));
}

vec3 importanceSampleGGX(vec2 Xi, vec3 N, float roughness)
{
    float a = roughness*roughness;

    float phi = 2.0 * 3.1415926535 * Xi.x;
    float cosTheta = sqrt((1.0 - Xi.y) / (1.0 + (a*a - 1.0) * Xi.y));
    float sinTheta = sqrt(1.0 - cosTheta*cosTheta);

    // from spherical coordinates to cartesian coordinates
    vec3 H;
    H.x = cos(phi) * sinTheta;
    H.y = sin(phi) * sinTheta;
    H.z = cosTheta;

    // from tangent-space vector to world-space sample vector
    vec3 up        = abs(N.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
    vec3 tangent   = normalize(cross(up, N));
    vec3 bitangent = cross(N, tangent);

    vec3 sampleVec = tangent * H.x + bitangent * H.y + N * H.z;
    return normalize(sampleVec);
}

void main()
{
   // outColor = texture(sampler2DArray(textures[pc.srcId], samp),
   //                             vec3(inUV,pc.srcLayer));

    vec3 localPos = sphericalToCartesian(inUV);

    vec3 N = normalize(localPos);
    vec3 R = N;
    vec3 V = R;

    const uint SAMPLE_COUNT = 1024u;
    float totalWeight = 0.0;
    vec3 prefilteredColor = vec3(0.0);
    for(uint i = 0u; i < SAMPLE_COUNT; ++i)
    {
        vec2 Xi = hammersley(i, SAMPLE_COUNT);
        vec3 H  = importanceSampleGGX(Xi, N, pc.roughness);
        vec3 L  = normalize(2.0 * dot(V, H) * H - V);

        float NdotL = max(dot(N, L), 0.0);
        if(NdotL > 0.0)
        {
            //prefilteredColor += texture(environmentMap, L).rgb * NdotL;
            prefilteredColor += texture(sampler2DArray(textures[pc.srcId], samp),
                                        vec3(sampleSphericalMap(L), pc.srcLayer)).rgb * NdotL;
            totalWeight      += NdotL;
        }
    }
    prefilteredColor = prefilteredColor / totalWeight;

    outColor = vec4(prefilteredColor, 1.0);
}

