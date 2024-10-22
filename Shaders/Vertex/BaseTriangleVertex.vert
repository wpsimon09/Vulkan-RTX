#version 460

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inUv;

void main() {
    gl_Position = vec4(inPosition, 1.0f);
}