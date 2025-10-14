#version 150

in vec4 vColor;
in vec3 vNormal;
in vec3 vViewPosition;

out vec4 oColor;

void main() {
    // Simple diffuse lighting
    vec3 lightDir = normalize(vec3(0.0, 0.0, -1.0));
    vec3 normal = normalize(vNormal);
    float diffuse = max(dot(normal, lightDir), 0.0);

    // Ambient + diffuse
    float ambient = 0.3;
    float lighting = ambient + diffuse * 0.7;

    oColor = vec4(vColor.rgb * lighting, vColor.a);
}
