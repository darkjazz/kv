#version 150

in vec4 ciPosition;   // world space: (wx, 0, wz)
in vec2 ciTexCoord0;  // UV [0,1]^2 for height texture lookup

uniform sampler2D uHeightTex;
uniform float     uPoolSize;
uniform float     uNormalScale;
uniform vec3      uLightDir;   // world-space light direction (need not be normalised)

out vec2 vOldPos;
out vec2 vNewPos;

void main() {
    float wx = ciPosition.x;
    float wz = ciPosition.z;
    vec2  uv = ciTexCoord0;

    float eps = 1.0 / 256.0;
    float hR  = texture(uHeightTex, uv + vec2(eps, 0.0)).r;
    float hL  = texture(uHeightTex, uv - vec2(eps, 0.0)).r;
    float hU  = texture(uHeightTex, uv + vec2(0.0, eps)).r;
    float hD  = texture(uHeightTex, uv - vec2(0.0, eps)).r;

    vec3 N = normalize(vec3(
        (hL - hR) * uNormalScale,
        2.0,
        (hD - hU) * uNormalScale
    ));

    vec3 I = normalize(uLightDir);
    vec3 refracted = refract(I, N, 0.75);
    if (refracted.y >= 0.0) refracted = I;

    float t = -uPoolSize / refracted.y;
    vec3 floorHit = vec3(wx, 0.0, wz) + refracted * t;
    floorHit.x = clamp(floorHit.x, -uPoolSize, uPoolSize);
    floorHit.z = clamp(floorHit.z, -uPoolSize, uPoolSize);

    vOldPos = vec2(wx, wz);
    vNewPos = vec2(floorHit.x, floorHit.z);

    gl_Position = vec4(floorHit.x / uPoolSize, floorHit.z / uPoolSize, 0.0, 1.0);
}
