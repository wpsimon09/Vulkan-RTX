#version 460

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inUv;

layout(binding = 0) uniform GlobalData {
    mat4 projection;
    mat4 view;
    vec4 lightPosition;
    vec4 cameraPostiion;

}ubo;

layout(binding = 1) uniform ModelMatrices{
    mat4 model;
    mat4 normalMatrix;
}ubo_model;

layout (location = 0) out vec3 normal;
layout (location = 1) out vec3 cameraPosition;
layout (location = 2) out vec3 fragPos;
layout (location = 3) out vec3 lightPos;
layout (location = 4) out vec2 uv;

void main() {
    gl_Position = ubo.view * ubo.projection * ubo_model.model * vec4(inPosition,1.0);
    normal = vec3(ubo_model.normalMatrix* vec4(inNormal,1.0));
    cameraPosition = ubo.cameraPostiion.xyz;
    fragPos = vec3(ubo_model.model * vec4(inPosition,1.0));
    lightPos = ubo.lightPosition.xyz;
    uv = inUv;
}