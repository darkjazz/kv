#version 150

uniform sampler2D uHeightTex;
uniform vec2  uCenter;
uniform float uRadius;
uniform float uStrength;

in  vec2 TexCoord;
out vec4 oColor;

const float PI = 3.14159265359;

void main() {
    vec4 info = texture(uHeightTex, TexCoord);

    float dist = length(TexCoord - uCenter) / max(uRadius, 0.001);
    float drop = max(0.0, 1.0 - dist);
    // Smooth cosine profile — no sharp discontinuity at the edge
    drop = 0.5 - cos(drop * PI) * 0.5;

    info.r += drop * uStrength;
    oColor = info;
}
