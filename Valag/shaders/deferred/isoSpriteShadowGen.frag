#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (constant_id = 0) const uint const_nbrRaySteps          = 256;
layout (constant_id = 1) const uint const_nbrSearchSteps       = 4;
layout (constant_id = 2) const float const_rayThreshold        = 15.0;

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

float insideBox(vec2 v, vec2 bottomRight, vec2 topLeft)
{
    vec2 s = step(bottomRight, v) - step(topLeft, v);
    return s.x * s.y;
}

vec4 sampleHeight(vec2 p)
{
    vec2 pn = (p - spritePos)/spriteSize.xy;

    return texture(sampler2DArray(textures[spriteHeightTexId.x], samp),
                                    vec3(pn,spriteHeightTexId.y)) * insideBox(pn, vec2(1.0), vec2(0.0));
}

vec2 raySearch(vec3 start, vec3 end, int under)
{
   vec3 cur = start;
   vec3 diff = end-start;
   float stp = 0.5;
   for(int i = 0 ; i < const_nbrSearchSteps ; ++i){
       vec3 test = cur + diff*stp;

        vec4 dstFrag    = sampleHeight(test.xy);
        float dstHeight = (dstFrag.x+dstFrag.y+dstFrag.z)*0.3333*dstFrag.a;//*spriteSize.z;

       if((1-2*under)*dstHeight < (1-2*under)*test.z) {
           cur = test; //Collision in second segment
       }
       stp = stp*0.5;
   }
   return cur.xy;
}

vec4 rayTrace(vec3 screenStart, vec3 ray)
{
    vec3 screenRayStep  = ray/float(const_nbrRaySteps);
    vec3 curPos         = screenStart;
    vec3 oldPos         = curPos;

    bool wasUnder   = true;
    bool isUnder    = false;

    vec4 dstFrag;
    float dstHeight;

    for(uint i = 0 ; i < const_nbrRaySteps ; ++i)
    {
        curPos     += screenRayStep;
        dstFrag     = sampleHeight(curPos.xy);
        dstHeight   = (dstFrag.x+dstFrag.y+dstFrag.z)*0.3333*dstFrag.a; //*spriteSize.z;

        if(dstFrag.a > 0.9)
        {
            if(curPos.z < dstHeight)
                isUnder = true;
            else
                isUnder = false;

            if(isUnder != wasUnder && abs(curPos.z - dstHeight) < const_rayThreshold )
            {
                curPos.xy   = raySearch(oldPos, curPos,int(wasUnder));
                dstFrag     = sampleHeight(curPos.xy);
                return dstFrag;
            }
        }

        oldPos      = curPos;
        wasUnder    = isUnder;
    }

    return vec4(0.0);
}

void main()
{
    vec3 ray    = viewLightDirection;
    //ray.z      *= spriteSize.z;
    outShadow   = rayTrace(vec3(inUV,0.0)-ray, ray);
}

