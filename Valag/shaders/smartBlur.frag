#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (constant_id = 0) const float radius             = 1.0;
layout (constant_id = 1) const float smartThresold      = 1.0;
layout (constant_id = 2) const bool  vertical           = false;

layout (set = 0, binding = 0) uniform sampler2D srcSampler;
layout (set = 0, binding = 1) uniform sampler2D samplerPositionOpac;
layout (set = 0, binding = 2) uniform sampler2D samplerPositionAlpha;

layout (location = 0) in vec2 inUV;

layout (location = 0) out vec4 outColor;


float gauss[5] = float[](
    float(.16),
    float(.15),
    float(.12),
    float(.09),
    float(.06)
);

float getSamplerPos(vec2 p)
{
    vec2 samplerOpac = texture(samplerPositionOpac, p).zw; //Opac pos.z and existence of truly transp pixel
    vec2 samplerAlpha = texture(samplerPositionAlpha, p).zw; //Alpha pos.z and opacity

    float mixing = (1.0 - samplerOpac.y) * samplerAlpha.y;

    //If the fragment is not covered by a truly transparent pixel, then we mix the values
    return samplerOpac.x * (1.0 - mixing) + samplerAlpha.x * mixing;
}


void main()
{
    vec2 offset = vec2(1-int(vertical), int(vertical)) * radius;

    outColor = texture(srcSampler, gl_FragCoord.xy) * gauss[0];
    float fragZ = getSamplerPos(gl_FragCoord.xy);

    float weight = gauss[0];
    vec2 p;
    float diffZ;

    for(uint i = 1 ; i < 5 ; ++i)
    {
        p = gl_FragCoord.xy+offset*0.25*i;
        diffZ = abs(getSamplerPos(p) - fragZ)/smartThresold;
        diffZ = clamp(2.0 - diffZ*2.0,0.0,1.0);
        //diffZ = step(diffZ, 1.0);

        {
            outColor += texture(srcSampler, p)	* gauss[i] * diffZ;
            weight   += gauss[i] * diffZ;
        }

        p = gl_FragCoord.xy-offset*0.25*i;
        diffZ = abs(getSamplerPos(p) - fragZ)/smartThresold;
        diffZ = clamp(2.0 - diffZ*2.0,0.0,1.0);
        //diffZ = step(diffZ, 1.0);

        {
            outColor += texture(srcSampler, p)	* gauss[i] * diffZ;
            weight   += gauss[i] * diffZ;
        }
    }

    outColor *= 1.0/weight;

    /*outColor =     (texture(srcSampler, gl_FragCoord.xy + offset * 1.0)	* 0.06 +
                    texture(srcSampler, gl_FragCoord.xy + offset * 0.75)	* 0.09 +
                    texture(srcSampler, gl_FragCoord.xy + offset * 0.5)	* 0.12 +
                    texture(srcSampler, gl_FragCoord.xy + offset * 0.25)	* 0.15 +
                    texture(srcSampler, gl_FragCoord.xy)	* 0.16 +
                    texture(srcSampler, gl_FragCoord.xy - offset * 1.0) 	* 0.06 +
                    texture(srcSampler, gl_FragCoord.xy - offset * 0.75)	* 0.09 +
                    texture(srcSampler, gl_FragCoord.xy - offset * 0.5)	* 0.12 +
                    texture(srcSampler, gl_FragCoord.xy - offset * 0.25)	* 0.15 );*/
}

