#version 150

// Scene rendered to FBO
uniform sampler2D uTexture;
// Water height field: r=height, g=velocity (b,a unused)
uniform sampler2D uWaterTex;

uniform float uIntensity;    // Overall caustic brightness (default 1.0)
uniform float uNormalScale;  // Amplify surface normals (default 3.0)
uniform float uDepth;        // Projection depth — how far light travels (default 0.4)
uniform float uWaterScale;   // Tile the water texture (default 1.0)
uniform vec3  uColor;        // Caustic colour tint (default 1.0, 0.95, 0.82)

in  vec2 TexCoord;
out vec4 oColor;

// Returns the UV where a refracted light ray originating at 'uv' lands on the floor.
vec2 refractedUV(vec2 uv) {
    vec2 waterUV = fract(uv * uWaterScale);
    float eps = 1.0 / 256.0;

    float h  = texture(uWaterTex, waterUV).r;
    float hx = texture(uWaterTex, waterUV + vec2(eps, 0.0)).r;
    float hy = texture(uWaterTex, waterUV + vec2(0.0, eps)).r;

    // Surface normal from finite differences
    vec3 N = normalize(vec3(-(hx - h) * uNormalScale,
                             1.0,
                            -(hy - h) * uNormalScale));

    // Refract straight-down incident ray through the surface (air→water, ratio 0.75)
    vec3 R = refract(vec3(0.0, -1.0, 0.0), N, 0.75);

    // XZ offset of the refracted ray projected to a floor at distance uDepth
    return uv + R.xz * uDepth;
}

void main() {
    vec4 scene = texture(uTexture, TexCoord);

    // Distorted UV for this pixel
    vec2 distUV = refractedUV(TexCoord);

    // Screen-space Jacobian of the distortion map
    vec2 dx = dFdx(distUV);
    vec2 dy = dFdy(distUV);
    float distortedArea = abs(dx.x * dy.y - dx.y * dy.x);

    // Reference: undistorted UV screen-space area
    vec2 udx = dFdx(TexCoord);
    vec2 udy = dFdy(TexCoord);
    float originalArea = abs(udx.x * udy.y - udx.y * udy.x);

    // Bright where rays converge (small distorted area relative to original).
    // Subtract 1 so flat water contributes nothing; clamp negatives.
    float caustic = originalArea / max(distortedArea, originalArea * 0.05) - 1.0;
    caustic = max(caustic, 0.0);
    caustic = pow(caustic, 1.4);  // Sharpen hot spots
    caustic *= uIntensity * 0.12;

    oColor   = scene + vec4(uColor * caustic, 0.0);
    oColor.a = scene.a;
}
