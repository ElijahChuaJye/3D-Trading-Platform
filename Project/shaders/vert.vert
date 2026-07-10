#version 450

// Hardcoded positions for a perfect fullscreen triangle
vec2 positions[3] = vec2[](
    vec2(0.0, -0.5),   // Top vertex
    vec2(0.5, 0.5),    // Bottom right vertex
    vec2(-0.5, 0.5)    // Bottom left vertex
);

void main() {
    // gl_VertexIndex automatically counts from 0 to 2 based on our vkCmdDraw(3) trigger
    gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);
}