#version 450
#extension GL_ARB_separate_shader_objects : enable


layout(binding = 0, set = 1) uniform sampler samp;
layout(binding = 1, set = 1) uniform texture2DArray textures[128];


layout(location = 0)        in vec2 inUV;
layout(location = 1) flat   in vec2 spriteSize;
layout(location = 2) flat   in uvec2 spriteAlbedoTexId;

layout(location = 0) out vec4 outShadow;

vec4 sampleTex(vec2 uv)
{
    return texture(sampler2DArray(textures[spriteAlbedoTexId.x], samp),
                                    vec3(uv,spriteAlbedoTexId.y));
}

float height(vec4 frag)
{
    return frag.a*(frag.r+frag.g+frag.b);
}

void main()
{
    vec2 texelSize = 1.0/spriteSize;

    vec4 maxFrag    = vec4(0.0);
    float maxHeight = 0;

    vec4 minFrag    = vec4(1.0);
    float minHeight = 3.0;

    vec4 frag       = sampleTex(inUV);
    float fragHeight = height(frag);

   // vec3 mean = frag.rgb*frag.a;

    int nbrBigger = 0;
    int nbrSmaller = 0;

    for(int x = -1 ; x <= 1 ; ++x) {
        for(int y = -1 ; y <= 1 ; ++y) {
            if(!(x == 0 && y == 0)) {
                vec4 samplingFrag = sampleTex(inUV  + vec2(x,y)*texelSize);
                float samplingHeight = height(samplingFrag);

                if(maxHeight < samplingHeight) {
                    maxFrag     = samplingFrag;
                    maxHeight   = samplingHeight;
                }

                if(minHeight > samplingHeight) {
                    minFrag     = samplingFrag;
                    minHeight   = samplingHeight;
                }

                if(samplingHeight - fragHeight > .2)
                    ++nbrBigger;
                else if(samplingHeight - fragHeight < -.2)
                    ++nbrSmaller;

                //mean += samplingFrag.rgb*samplingFrag.a;
            }
        }
    }

    outShadow = frag;

    //Could use clamp to avoid branch
    if(nbrBigger > 4)
        outShadow = maxFrag;
    if(nbrSmaller > 4)
        outShadow = minFrag;
}

