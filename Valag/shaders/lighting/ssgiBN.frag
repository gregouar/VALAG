#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (constant_id = 0) const float const_rayLength          = 25.0;
layout (constant_id = 1) const float const_rayLengthFactor    = 0.0;
layout (constant_id = 2) const float const_rayThreshold        = 15.0;
layout (constant_id = 3) const float const_rayThresholdFactor  = 0.0;

layout(set = 0, binding = 0) uniform ViewUBO {
    mat4 view;
    mat4 viewInv;
    vec2 screenOffset;
    vec2 screenSizeFactor;
    vec2 depthOffsetAndFactor;
} viewUbo;

layout (set = 1, binding = 0) uniform sampler2D samplerPosition;
layout (set = 1, binding = 1) uniform sampler2D samplerNormal;


layout (location = 0) in vec2 inUV;

layout (location = 0) out vec4 outBentNormal;
layout (location = 1) out vec4 outCollision1;
layout (location = 2) out vec4 outCollision2;
layout (location = 3) out vec4 outCollision3;
layout (location = 4) out vec4 outCollision4;

//Maybe 4 raycast per pixel is already too much...

layout(push_constant) uniform PER_OBJECT
{
	int imgIndex;
}pc;

vec3 samplesHemisphere[16] = vec3[](
    /*vec3(0,0,.8),
    vec3(0,0,.4),
    vec3(0,0,.2),
    vec3(0,0,.1),
    vec3(1,0,.4),
    vec3(-1,0,.4),
    vec3(0,1,.4),
    vec3(0,-1,.4),
    vec3(.5,.5,.5),
    vec3(.5,-.5,.5),
    vec3(-.5,.5,.5),
    vec3(-.5,-.5,.5),
    vec3(.5,0,.5),
    vec3(-.5,0,.5),
    vec3(0,.5,.5),
    vec3(0,-.5,.5)*/
    /*vec3(.4,0,.8),
    vec3(0,.2,.4),
    vec3(.1,0,.2),
    vec3(0,0,.1),
    vec3(1,0,.4),
    vec3(-1,0,.4),
    vec3(0,1,.4),
    vec3(0,-1,.4),
    vec3(.5,.5,.5),
    vec3(.5,-.5,.5),
    vec3(-.5,.5,.5),
    vec3(-.5,-.5,.5),
    vec3(.5,0,.5),
    vec3(-.5,0,.5),
    vec3(0,.5,.5),
    vec3(0,-.5,.5)*/
vec3(0.0477973,0.022405,0.140405),
vec3(-0.0477973,-0.022405,0.140405),
vec3(-0.151491,-0.107032,0.0411661),
vec3(0.151491,0.107032,0.0411661),
vec3(-0.501778,-0.528188,0.0462164),
vec3(0.501778,0.528188,0.0462164),
vec3(-0.0118434,-0.331614,0.240815),
vec3(0.0118434,0.331614,0.240815),
vec3(0.710307,-0.1215,0.168231),
vec3(-0.710307,0.1215,0.168231),
vec3(0.368779,-0.652455,0.0283676),
vec3(-0.368779,0.652455,0.0283676),
vec3(0.303026,0.264267,0.345309),
vec3(-0.303026,-0.264267,0.345309),
vec3(0.522069,-0.422261,0.614198),
vec3(-0.522069,0.422261,0.614198)
);

vec2 hashed[16] = vec2[](
    vec2(.92, .09),
    vec2(-.51,-.10),
    vec2(-.70,.22),
    vec2(-.97, .03),
    vec2(-.42, .75),
    vec2(.64,1.0),
    vec2(-.56,.86),
    vec2(.45,-.15),
    vec2(.21,.67),
    vec2(.46,.07),
    vec2(.65, -.57),
    vec2(-.88, .6),
    vec2(-.53,.12),
    vec2(.71,.08),
    vec2(.05,-.82),
    vec2(-.4,-.14)
);


//This should be replace by something better
vec3 hash(vec3 a)
{
   a = fract(a*0.8);
   a += dot(a, a.xyz + 19.19);
   return fract((a.xxy + a .yxx) * a.zyx);
}


vec3 rayTrace(vec3 screenStart, vec3 ray)
{
    uint nbrSteps   = 4;

    vec3 screenRayStep  = vec3(vec4(viewUbo.view * vec4(ray ,0.0)).xy, ray.z) /*/float(nbrSteps)*/;
    vec3 curPos         = screenStart;

    bool wasUnder   = false;
    bool isUnder    = false;

    for(uint i = 0 ; i < nbrSteps ; ++i)
    {
        curPos += screenRayStep*(i*const_rayLengthFactor+1);
        float dstFragHeight = texture(samplerPosition, curPos.xy*viewUbo.screenSizeFactor.xy*0.5).z;

        if(curPos.z < dstFragHeight)
            isUnder = true;
        else
            isUnder = false;

        if(isUnder != wasUnder
        && (curPos.z - dstFragHeight) > (1-2*int(isUnder)) * const_rayThreshold *(i*const_rayThresholdFactor+1.0)
        && (curPos.z - dstFragHeight) < (1-2*int(isUnder)) * -.01)
            return vec3(curPos.xy,/*abs(curPos.z - dstFragHeight)/15.0*/i);

        //wasUnder = isUnder;
    }

    return vec3(-1);
}


void main()
{
    float fragHeight    = texture(samplerPosition, inUV).z;
    vec3  fragNormal    = normalize(texture(samplerNormal, inUV).xyz);

    uint d = uint(gl_FragCoord.x) % 2 + 2*(uint(inUV)%2);
    uint dp = uint(gl_FragCoord.x) % 4 + 4*(uint(inUV)%4);
    //d = (d + uint(pc.imgIndex)) % 4;

    //d = 1;

    //vec3 rVec   = vec3(hash(vec3(gl_FragCoord.xy+vec2(pc.imgIndex,pc.imgIndex),d)).xy, 1.0);
    vec3 rVec   = vec3(hashed[/*pc.imgIndex % 4 + d*4*/dp].xy, 0.0);
	vec3 t      = normalize(rVec - fragNormal * dot(rVec, fragNormal));
	mat3 rot    = mat3(t,cross(fragNormal,t),fragNormal);

    //outBentNormal = vec4(fragNormal/* *0.2 */, 1.0);
    outBentNormal = vec4(vec3(0.0), 1.0);
    //outBentNormal = vec4(fragNormal, 1.0);

    outCollision1 = vec4(0.0);
    outCollision2 = vec4(0.0);
    outCollision3 = vec4(0.0);
    outCollision4 = vec4(0.0);

    /** I could do à la Starcraft : use half rays for close (with indpt averaging) and half for longer shots, yay**/
    //store in outBentNormal.a the local SSAO value and in |xyz| the GI value !

    float ao    = 1.0;
    float gio   = 1.0;

    uint j = 0;
    for(uint i = 0 ; i < 16 ; ++i)
    {
        vec3 ray = const_rayLength * (rot * samplesHemisphere[/*d*4+*/(i+pc.imgIndex)%16]);
        //vec3 ray = fragNormal*15.0;

        vec3 c = rayTrace(vec3(inUV, fragHeight), ray);

        if(c.z != -1)
        {
            outBentNormal.xyz += normalize(ray)*c.z;
            if(j == 0) outCollision1 = vec4(c, 0.0);
            if(j == 1) outCollision2 = vec4(c, 0.0);
            if(j == 2) outCollision3 = vec4(c, 0.0);
            if(j == 3) outCollision4 = vec4(c, 0.0);

            ao  -= 1.0/16.0 * max(1.0-c.z, 0.0);
            gio -= 1.0/16.0;//(4.0-c.z)/(4.0 * 16.0);

            j++;
        }
        else
            outBentNormal.xyz += normalize(ray)*6.0;
    }

    gio = 0.01+gio*0.99;

    if(outBentNormal.z == 0)
        outBentNormal.z = 0.0001;

    outBentNormal.xyz = normalize(outBentNormal.xyz); // * (gio*0.9 + 0.1);
    outBentNormal.z = (1.0-gio) * sign(outBentNormal.z);
    outBentNormal.a = ao;

}

