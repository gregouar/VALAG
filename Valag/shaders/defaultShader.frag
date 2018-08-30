#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0, set = 2) uniform ModelUBO {
    mat4 model;
    vec4 color;
} modelUbo;

//layout(location = 0) in vec4 fragColor;
layout(location = 0) in vec2 fragTexCoord;

layout(binding = 0, set = 1) uniform sampler samp;
layout(binding = 1, set = 1) uniform texture2DArray textures[128];

layout(push_constant) uniform PER_OBJECT
{
	int texId;
    int texLayer;
}pc;

layout(location = 0) out vec4 outColor;

void main()
{
    outColor = texture(sampler2DArray(textures[pc.texId], samp), vec3(fragTexCoord,pc.texLayer)) * modelUbo.color;
    //vec4(fragColor, 1.0);
}
