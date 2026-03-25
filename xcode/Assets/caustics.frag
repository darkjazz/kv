#version 150

// Scene rendered to FBO
uniform sampler2D uTexture;
// Water height field: r=height, g=velocity (b,a unused)
uniform sampler2D uWaterTex;

uniform float uIntensity;    // Overall caustic brightness (default 1.0)
uniform float uNormalScale;  // Laplacian scale → caustic brightness (default 1.0)
uniform float uDepth;        // Scene warp amount from height gradient (default 0.08)
uniform float uWaterScale;   // Tile the water texture (default 1.0)
uniform vec3  uColor;        // Caustic colour tint (default 1.0, 0.95, 0.82)

in  vec2 TexCoord;
out vec4 oColor;

void main() {
    vec2 waterUV = fract(TexCoord * uWaterScale);
    float eps = 1.0 / 256.0;

    // 5-tap height samples
    float h  = texture(uWaterTex, waterUV).r;
    float hl = texture(uWaterTex, waterUV - vec2(eps, 0.0)).r;
    float hr = texture(uWaterTex, waterUV + vec2(eps, 0.0)).r;
    float hd = texture(uWaterTex, waterUV - vec2(0.0, eps)).r;
    float hu = texture(uWaterTex, waterUV + vec2(0.0, eps)).r;

    // Gradient: use to warp scene UV (simulates looking through rippled water)
    vec2 grad  = vec2(hr - hl, hu - hd) * uDepth * 8.0;
    vec2 sceneUV = clamp(TexCoord + grad, 0.001, 0.999);
    vec4 scene = texture(uTexture, sceneUV);

    // Laplacian of the height field — positive = concave surface = focuses light
    float lap = (hl + hr + hd + hu) - 4.0 * h;

    // Only concave areas produce bright caustics; Reinhard prevents blowout
    float caustic = max(lap * uNormalScale * 250.0, 0.0);
    caustic = caustic / (caustic + 1.0);   // Reinhard: always in [0, 1)
    caustic *= uIntensity;

    oColor   = scene + vec4(uColor * caustic * 0.45, 0.0);
    oColor.a = scene.a;
}
