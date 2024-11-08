#version 460

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inUv;

layout(binding = 0) uniform GlobalData {
    mat4 projection;
    mat4 view;
}ubo;

layout(binding = 1) uniform ModelMatrices{
    mat4 model;
}ubo_model;


void main() {
    gl_Position = ubo.view * ubo.projection * vec4(inPosition,1.0);
}