#version 150

in vec3 vWorldPos;
in vec3 vNormal;

uniform vec3  uWaterColor;  // base water colour (default 0.1, 0.4, 0.7)
uniform float uAlpha;       // transparency (default 0.65)
uniform vec3  uCameraPos;   // for specular highlight

out vec4 oColor;

void main() {
    vec3 N = normalize(vNormal);
    vec3 L = vec3(0.0, 1.0, 0.0);  // light from above

    float diff = max(dot(N, L), 0.15);

    // Blinn-Phong specular
    vec3 V = normalize(uCameraPos - vWorldPos);
    vec3 H = normalize(L + V);
    float spec = pow(max(dot(N, H), 0.0), 64.0) * 0.6;

    vec3 color = uWaterColor * diff + vec3(spec);
    oColor = vec4(color, uAlpha);
}
