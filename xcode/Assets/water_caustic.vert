#version 150

// UV grid position on water surface [0,1]^2
in vec2 ciTexCoord0;

uniform sampler2D uHeightTex;   // r=height, g=velocity
uniform float     uPoolSize;    // half-extent in world units
uniform float     uNormalScale; // maps height gradient to surface normal tilt

// Passed to fragment for area-distortion brightness
out vec2 vOldPos;   // straight-down floor hit (XZ, world)
out vec2 vNewPos;   // refracted floor hit (XZ, world)

void main() {
    vec2 uv = ciTexCoord0;

    // World position on water surface (Y=0)
    float wx = (uv.x - 0.5) * 2.0 * uPoolSize;
    float wz = (uv.y - 0.5) * 2.0 * uPoolSize;

    // Height field normal — sample with explicit LOD (vertex stage, no auto mip)
    float eps = 1.0 / 256.0;
    float hR  = texture(uHeightTex, uv + vec2(eps, 0.0)).r;
    float hL  = texture(uHeightTex, uv - vec2(eps, 0.0)).r;
    float hU  = texture(uHeightTex, uv + vec2(0.0, eps)).r;
    float hD  = texture(uHeightTex, uv - vec2(0.0, eps)).r;

    // Y-up normal from height gradient (scale 2.0 keeps it stable when calm)
    vec3 N = normalize(vec3(
        (hL - hR) * uNormalScale,
        2.0,
        (hD - hU) * uNormalScale
    ));

    // Light travels straight down into water
    vec3 I = vec3(0.0, -1.0, 0.0);

    // Refract: air→water, IOR ratio = 1/1.333 ≈ 0.75
    vec3 refracted = refract(I, N, 0.75);

    // If total internal reflection (refracted.y >= 0), use straight-down ray
    if (refracted.y >= 0.0) refracted = I;

    // Floor at Y = -uPoolSize (pool depth == half-extent)
    float t = -uPoolSize / refracted.y;
    vec3 floorHit = vec3(wx, 0.0, wz) + refracted * t;

    // Clamp to pool bounds
    floorHit.x = clamp(floorHit.x, -uPoolSize, uPoolSize);
    floorHit.z = clamp(floorHit.z, -uPoolSize, uPoolSize);

    // Straight-down reference (no refraction)
    vOldPos = vec2(wx, wz);
    vNewPos = vec2(floorHit.x, floorHit.z);

    // Project refracted floor hit to caustic FBO NDC
    // XZ [-poolSize..+poolSize] → [-1..+1]
    gl_Position = vec4(floorHit.x / uPoolSize, floorHit.z / uPoolSize, 0.0, 1.0);
}
