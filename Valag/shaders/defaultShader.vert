#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0, set = 0) uniform ViewUBO {
    mat4 view;
    vec2 screenOffset;
    vec2 screenSizeFactor;
    vec2 depthOffsetAndFactor;
} viewUbo;

layout(binding = 0, set = 2) uniform ModelUBO {
    mat4 model;
    vec4 color;
    vec2 texPos;
    vec2 texExt;
} modelUbo;

layout(location = 0) out vec2 fragTexCoord;

vec2 vertPos[4] = vec2[](
    vec2(0.0, 1.0),
    vec2(0.0, 0.0),
    vec2(1.0, 1.0),
    vec2(1.0, 0.0)
);

out gl_PerVertex {
    vec4 gl_Position;
};

void main() {
    gl_Position =  viewUbo.view * modelUbo.model *  vec4(vertPos[gl_VertexIndex], 0.0, 1.0);
    gl_Position.xyz = gl_Position.xyz * vec3(viewUbo.screenSizeFactor, viewUbo.depthOffsetAndFactor.y)
                        + vec3(viewUbo.screenOffset, viewUbo.depthOffsetAndFactor.x);
    fragTexCoord = modelUbo.texExt * vertPos[gl_VertexIndex] + modelUbo.texPos;
}



/**#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0, set = 0) uniform ViewUBO {
    mat4 view;
} viewUbo;

layout(binding = 0, set = 2) uniform ModelUBO {
    mat4 model;
    vec4 color;
} modelUbo;

layout(location = 0) in vec2 inPosition;
//layout(location = 1) in vec4 inColor;
layout(location = 1) in vec2 inTexCoord;

//layout(location = 0) out vec4 fragColor;
layout(location = 0) out vec2 fragTexCoord;

out gl_PerVertex {
    vec4 gl_Position;
};

void main() {
    gl_Position =  viewUbo.view * modelUbo.model *  vec4(inPosition, 0.0, 1.0);
    //fragColor = inColor;
    fragTexCoord = inTexCoord;
}**/
