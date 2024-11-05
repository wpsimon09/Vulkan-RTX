#version 460

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inUv;

layout(set=0, binding = 0) uniform UnifromBufferObject{
    mat4 projection;
    mat4 view;
}ubo;


void main() {
    gl_Position =  ubo.projection * ubo.view * mat4(1.0f) * vec4(inPosition,1.0);
}