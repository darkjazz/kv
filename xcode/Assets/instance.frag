#version 150

in vec4 vColor;
in vec3 vNormal;
in vec3 vViewPosition;

out vec4 oColor;

void main() {
    // Multi-light setup for better visibility
    vec3 normal = normalize(vNormal);

    // Key light (from camera direction)
    vec3 keyLight = normalize(vec3(0.5, 0.5, -1.0));
    float keyDiffuse = max(dot(normal, keyLight), 0.0);

    // Fill light (from opposite side)
    vec3 fillLight = normalize(vec3(-0.5, 0.2, -0.5));
    float fillDiffuse = max(dot(normal, fillLight), 0.0) * 0.4;

    // Rim light (from behind)
    vec3 rimLight = normalize(vec3(0.0, 1.0, 1.0));
    float rimDiffuse = max(dot(normal, rimLight), 0.0) * 0.3;

    // Strong ambient to ensure visibility
    float ambient = 0.5;
    float lighting = ambient + keyDiffuse * 0.7 + fillDiffuse + rimDiffuse;

    // Clamp to prevent overexposure
    lighting = min(lighting, 1.5);

    oColor = vec4(vColor.rgb * lighting, vColor.a);
}
