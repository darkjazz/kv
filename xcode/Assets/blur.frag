#version 150

uniform sampler2D uTexture;
uniform float uBlurAmount;  // Blur intensity 0.0 - 1.0
uniform vec2 uTexelSize;    // 1.0 / texture size

in vec2 TexCoord;
out vec4 oColor;

// Simple 9-tap gaussian blur
void main() {
    vec4 color = vec4(0.0);

    if (uBlurAmount <= 0.0) {
        // No blur - passthrough
        oColor = texture(uTexture, TexCoord);
        return;
    }

    // Blur radius based on amount
    float radius = uBlurAmount * 5.0;
    vec2 offset = uTexelSize * radius;

    // 9-tap gaussian kernel (approximate)
    color += texture(uTexture, TexCoord + vec2(-offset.x, -offset.y)) * 0.05;
    color += texture(uTexture, TexCoord + vec2(0.0, -offset.y)) * 0.09;
    color += texture(uTexture, TexCoord + vec2(offset.x, -offset.y)) * 0.05;

    color += texture(uTexture, TexCoord + vec2(-offset.x, 0.0)) * 0.09;
    color += texture(uTexture, TexCoord) * 0.44;  // Center weight
    color += texture(uTexture, TexCoord + vec2(offset.x, 0.0)) * 0.09;

    color += texture(uTexture, TexCoord + vec2(-offset.x, offset.y)) * 0.05;
    color += texture(uTexture, TexCoord + vec2(0.0, offset.y)) * 0.09;
    color += texture(uTexture, TexCoord + vec2(offset.x, offset.y)) * 0.05;

    oColor = color;
}
