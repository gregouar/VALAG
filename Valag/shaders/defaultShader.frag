#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0, set = 2) uniform ModelUBO {
    mat4 model;
    vec4 color;
} modelUbo;

//layout(location = 0) in vec4 fragColor;
layout(location = 0) in vec2 fragTexCoord;

layout(binding = 0, set = 1) uniform sampler samp;
layout(binding = 1, set = 1) uniform texture2D textures[128];

layout(push_constant) uniform PER_OBJECT
{
	int texId;
}pc;

layout(location = 0) out vec4 outColor;

void main()
{
    outColor = texture(sampler2D(textures[pc.texId], samp), fragTexCoord) * modelUbo.color;
    //vec4(fragColor, 1.0);
}
