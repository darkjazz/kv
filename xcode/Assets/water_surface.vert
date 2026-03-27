#version 150

in vec4 ciPosition;   // world space: (wx, 0, wz)
in vec2 ciTexCoord0;  // UV [0,1]^2 for height texture lookup

uniform sampler2D uHeightTex;
uniform mat4      ciModelViewProjection;
uniform float     uHeightScale;
uniform float     uNormalScale;

out vec3 vWorldPos;
out vec3 vNormal;

void main() {
    float wx = ciPosition.x;
    float wz = ciPosition.z;
    vec2  uv = ciTexCoord0;

    float h = texture(uHeightTex, uv).r * uHeightScale;

    vWorldPos = vec3(wx, h, wz);

    float eps = 1.0 / 128.0;
    float hR  = texture(uHeightTex, uv + vec2(eps, 0.0)).r * uHeightScale;
    float hL  = texture(uHeightTex, uv - vec2(eps, 0.0)).r * uHeightScale;
    float hU  = texture(uHeightTex, uv + vec2(0.0, eps)).r * uHeightScale;
    float hD  = texture(uHeightTex, uv - vec2(0.0, eps)).r * uHeightScale;

    vNormal = normalize(vec3(
        (hL - hR) * uNormalScale,
        2.0,
        (hD - hU) * uNormalScale
    ));

    gl_Position = ciModelViewProjection * vec4(wx, h, wz, 1.0);
}
