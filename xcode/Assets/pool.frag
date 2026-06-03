#version 150

in vec3 vWorldPos;
in vec3 vNormal;

uniform sampler2D uCausticTex;      // top-down caustic projection FBO
uniform vec3      uPoolColor;       // base tile colour
uniform float     uPoolSize;        // half-extent
uniform float     uCausticStrength; // caustic add-on
uniform float     uAmbient;         // ambient light level
uniform vec3      uLightDir;        // world-space light direction (unnormalised OK)

out vec4 oColor;

void main() {
    // Sample caustic texture: world XZ → [0,1] UV (works for floor and walls)
    vec2 cUV = (vWorldPos.xz / uPoolSize) * 0.5 + 0.5;
    cUV = clamp(cUV, 0.001, 0.999);
    vec3 caustic = texture(uCausticTex, cUV).rgb;

    // Diffuse using the scene light direction — illuminates walls as well as floor
    vec3  N       = normalize(vNormal);
    vec3  toLight = normalize(-uLightDir);
    float diff    = max(dot(N, toLight), 0.0);
    float light   = uAmbient + (1.0 - uAmbient) * diff;

    // Caustics only on upward-facing surfaces (floor); walls get none
    float causticFactor = max(dot(N, vec3(0.0, 1.0, 0.0)), 0.0);
    vec3 color = uPoolColor * light + caustic * uCausticStrength * causticFactor;
    oColor = vec4(color, 1.0);
}
