#version 460


layout(location = 0) in vec3 outFragColor;
layout(location = 0) out vec4 FragColor;

void main() {
    vec2 coord = gl_PointCoord - vec2(0.5);
    FragColor = vec4(outFragColor.rgb, 1.0);
}