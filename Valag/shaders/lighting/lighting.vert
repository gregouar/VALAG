#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0, set = 0) uniform ViewUBO {
    mat4 view;
    mat4 viewInv;
    vec2 screenOffset;
    vec2 screenSizeFactor;
    vec2 depthOffsetAndFactor;
} viewUbo;

layout(push_constant) uniform PER_OBJECT
{
    vec4 camPosAndZoom;
}pc;

layout(location = 0) in vec4  inPos;
layout(location = 1) in vec4  inColor;
layout(location = 2) in float inRadius;

layout(location = 0) flat out vec4  lightPos;
layout(location = 1) flat out vec3  lightColor;
layout(location = 2) flat out float lightRadiusInv;


//2/sqrt(3) ~ 1.1547
//1/sqrt(3) ~ 0.57735

vec2 vertPos[8] = vec2[](
    vec2( 0.0       , 0.0),
    vec2( 1.1547    , 0.0),
    vec2( 0.57735   , 1.0),
    vec2(-0.57735   , 1.0),
    vec2(-1.1547    , 0.0),
    vec2(-0.57735   ,-1.0),
    vec2( 0.57735   ,-1.0),
    vec2( 1.1547    , 0.0)
);

out gl_PerVertex
{
	vec4 gl_Position;
};

void main()
{
    vec4 screenPos = viewUbo.view*(inPos-vec4(pc.camPosAndZoom.xyz,0));

    if(inPos.w == 0.0)
    {
        vec2 outUV = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2);
        if(gl_VertexIndex > 2) outUV = vec2(0.0);
        gl_Position = vec4(outUV * 2.0f - 1.0f, 0.0f, 1.0f);
    } else {
        gl_Position = vec4(screenPos.xyz + vec3(vertPos[gl_VertexIndex] * inRadius * (1.0 - exp(-inColor.a*0.2-1)), 0.0), 1.0);
        gl_Position.xyz = gl_Position.xyz * vec3(viewUbo.screenSizeFactor, viewUbo.depthOffsetAndFactor.y)
                            + vec3(viewUbo.screenOffset, viewUbo.depthOffsetAndFactor.x);
    }


    lightPos    = inPos;
    lightColor  = inColor.rgb*inColor.a;
    lightRadiusInv = 1.0/(inRadius*0.01);
}


