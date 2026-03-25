#version 150

in vec3 vWorldPos;
in vec3 vNormal;

uniform sampler2D uCausticTex;      // top-down caustic projection FBO
uniform vec3      uPoolColor;       // base tile colour (default 0.15, 0.15, 0.2)
uniform float     uPoolSize;        // half-extent
uniform float     uCausticStrength; // caustic add-on (default 0.8)
uniform float     uAmbient;         // ambient light level (default 0.15)

out vec4 oColor;

void main() {
    // Sample caustic texture: world XZ → [0,1] UV
    // Matches the caustic FBO projection: NDC x = worldX/poolSize
    vec2 cUV = (vWorldPos.xz / uPoolSize) * 0.5 + 0.5;
    cUV = clamp(cUV, 0.001, 0.999);
    vec3 caustic = texture(uCausticTex, cUV).rgb;

    // Diffuse from overhead light (0,1,0)
    float diff = max(dot(normalize(vNormal), vec3(0.0, 1.0, 0.0)), 0.0);
    float light = uAmbient + (1.0 - uAmbient) * diff;

    vec3 color = uPoolColor * light + caustic * uCausticStrength;
    oColor = vec4(color, 1.0);
}
