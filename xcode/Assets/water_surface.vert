#version 150

// UV grid position on water surface [0,1]^2
in vec2 ciTexCoord0;

uniform sampler2D uHeightTex;
uniform mat4      ciModelViewProjection;  // Cinder supplies P*V (model=identity)
uniform float     uPoolSize;
uniform float     uHeightScale;   // visual displacement scale (default 0.5)
uniform float     uNormalScale;   // gradient to normal scale

out vec3 vWorldPos;
out vec3 vNormal;

void main() {
    vec2 uv = ciTexCoord0;

    float wx = (uv.x - 0.5) * 2.0 * uPoolSize;
    float wz = (uv.y - 0.5) * 2.0 * uPoolSize;

    // Height displacement (explicit LOD in vertex stage)
    float h = textureLod(uHeightTex, uv, 0.0).r * uHeightScale;

    vWorldPos = vec3(wx, h, wz);

    // Normal from gradient
    float eps = 1.0 / 128.0;
    float hR  = textureLod(uHeightTex, uv + vec2(eps, 0.0), 0.0).r * uHeightScale;
    float hL  = textureLod(uHeightTex, uv - vec2(eps, 0.0), 0.0).r * uHeightScale;
    float hU  = textureLod(uHeightTex, uv + vec2(0.0, eps), 0.0).r * uHeightScale;
    float hD  = textureLod(uHeightTex, uv - vec2(0.0, eps), 0.0).r * uHeightScale;

    vNormal = normalize(vec3(
        (hL - hR) * uNormalScale,
        2.0,
        (hD - hU) * uNormalScale
    ));

    gl_Position = ciModelViewProjection * vec4(wx, h, wz, 1.0);
}
