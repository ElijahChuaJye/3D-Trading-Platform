#version 450

// layout(location = 0) targets our color swapchain image channel
layout(location = 0) out vec4 outColor;

void main() {
    // Output a solid bright color to make sure it renders beautifully
    // Let's use a nice neon teal/cyan: Red=0.0, Green=1.0, Blue=0.8, Alpha=1.0
    outColor = vec4(0.0, 1.0, 0.8, 1.0);
}