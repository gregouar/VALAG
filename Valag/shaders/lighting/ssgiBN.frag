#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (set = 0, binding = 0) uniform sampler2D samplerPosition;
layout (set = 0, binding = 1) uniform sampler2D samplerNormal;

layout (location = 0) in vec2 inUV;

layout (location = 0) out vec4 outBentNormal;
layout (location = 1) out vec4 outCollision1;
layout (location = 2) out vec4 outCollision2;
layout (location = 3) out vec4 outCollision3;
layout (location = 4) out vec4 outCollision4;

//Maybe 4 raycast per pixel is too much...

void main()
{
    vec3 fragPos    = texture(samplerPosition, gl_FragCoord.xy).xyz;
    vec3 fragNormal = normalize(texture(samplerNormal, gl_FragCoord.xy).xyz);

    outBentNormal = vec4(fragNormal, 1.0);

    outCollision1 = vec4(0.0);
    outCollision2 = vec4(0.0);
    outCollision3 = vec4(0.0);
    outCollision4 = vec4(0.0);
}

/*
"uniform sampler2D map_normal;" \
    "uniform sampler2D map_depth;" \
    "uniform sampler2D map_noise;" \
    "uniform mat3 p_isoToCartMat;" \
    "uniform vec2 view_ratio;" \
    "uniform float view_zoom;"
    "uniform vec3 p_samplesHemisphere[16];"
    "void main()" \
    "{" \
    "   const vec3 constantList = vec3(1.0, 0.0, -1.0);"
    "   vec3 normalPixel = texture2D(map_normal, gl_TexCoord[0].xy).rgb;" \
    "   vec3 depthPixel = texture2D(map_depth, gl_TexCoord[0].xy).rgb;" \
    "   vec3 direction = 2.0*normalPixel.rgb-1.0;"
    "   float heightPixel = (depthPixel.b*"<<1.0/255.0<<"+depthPixel.g)*"<<1.0/255.0<<"+depthPixel.r;"
    "   float occlusion = 12.0;"
    "   vec3 rVec = vec3(2.0*texture2D(map_noise, gl_FragCoord.xy * "<<1.0/SSAO_SCREEN_RATIO<<"*0.25).rg-1.0, 1.0);"
	"   vec3 t = normalize(rVec - direction * dot(rVec, direction));"
	"   mat3 rot = mat3(t,cross(direction,t),direction);"
	"   for(int i =0 ; i < 16 ; ++i){"
	"       vec3 rayShift = (rot * p_samplesHemisphere[i]) ;"
	"       vec2 screenShift = (view_zoom* 15.0)*(rayShift*p_isoToCartMat).xy;"
	"       rayShift.z *= "<<15.0*PBRTextureAsset::DEPTH_BUFFER_NORMALISER<<";"
	"       vec2 screenPos = gl_FragCoord.xy * "<<1.0/SSAO_SCREEN_RATIO<<"  + screenShift * constantList.xz;"
	"       vec3 occl_depthPixel = texture2D(map_depth, screenPos*view_ratio).rgb;"
	"       float occl_height = (occl_depthPixel.b*"<<1.0/255.0<<"+occl_depthPixel.g)*"<<1.0/255.0<<"+occl_depthPixel.r;"
    "       if(occl_height < (heightPixel-rayShift.z) - "<<0.1*PBRTextureAsset::DEPTH_BUFFER_NORMALISER<<"  "
    "        && occl_height - (heightPixel-rayShift.z) > "<<-20*PBRTextureAsset::DEPTH_BUFFER_NORMALISER<<")"
    "           --occlusion;"
	"   } "
    "   gl_FragColor.rgb = constantList.xxx*pow(occlusion*"<<1.0/12.0<<","<<SSAO_STRENGTH<<");" \
    "   gl_FragColor.a = 1;" \
    "}";*/
