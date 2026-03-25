#version 150

uniform sampler2D uHeightTex;
uniform vec2  uDelta;    // 1.0 / size
uniform float uDamping;
uniform float uSpeed;

in  vec2 TexCoord;
out vec4 oColor;

void main() {
    vec4 info = texture(uHeightTex, TexCoord);

    // 4-neighbour Laplacian average
    float left  = texture(uHeightTex, TexCoord - vec2(uDelta.x, 0.0)).r;
    float right = texture(uHeightTex, TexCoord + vec2(uDelta.x, 0.0)).r;
    float down  = texture(uHeightTex, TexCoord - vec2(0.0, uDelta.y)).r;
    float up    = texture(uHeightTex, TexCoord + vec2(0.0, uDelta.y)).r;
    float avg   = (left + right + down + up) * 0.25;

    // Wave equation: velocity accelerates toward neighbourhood average
    info.g += (avg - info.r) * uSpeed;
    info.g *= uDamping;
    info.r += info.g;

    oColor = info;
}
