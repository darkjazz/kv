#version 150

uniform sampler2D uHeightTex;
uniform vec2  uDelta;    // 1.0 / size
uniform float uDamping;
uniform float uSpeed;

// Corner boundary forcing (Dirichlet): set height = audio band value each frame
uniform bool  uBoundaryEnabled;
uniform vec4  uBoundaryValues;  // (sub, low, mid, high) band amplitudes
uniform float uBoundaryRadius;  // corner region radius in UV space (e.g. 0.05)

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

    // Boundary forcing: override corners with live audio values
    if (uBoundaryEnabled) {
        vec2  corners[4];
        corners[0] = vec2(0.0, 0.0);
        corners[1] = vec2(1.0, 0.0);
        corners[2] = vec2(0.0, 1.0);
        corners[3] = vec2(1.0, 1.0);
        for (int i = 0; i < 4; i++) {
            if (distance(TexCoord, corners[i]) < uBoundaryRadius) {
                info.r = uBoundaryValues[i];
                info.g = 0.0;
            }
        }
    }

    oColor = info;
}
