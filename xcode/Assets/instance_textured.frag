#version 150

uniform sampler2D uTexture;

in vec4 vColor;
in vec3 vNormal;
in vec3 vViewPosition;
in vec2 vTexCoord;

out vec4 oColor;

void main() {
    vec3 normal = normalize(vNormal);

    // Three-point lighting
    vec3 keyLight = normalize(vec3(0.5, 0.5, -1.0));
    float keyDiffuse = max(dot(normal, keyLight), 0.0);

    vec3 fillLight = normalize(vec3(-0.5, 0.2, -0.5));
    float fillDiffuse = max(dot(normal, fillLight), 0.0) * 0.4;

    vec3 rimLight = normalize(vec3(0.0, 1.0, 1.0));
    float rimDiffuse = max(dot(normal, rimLight), 0.0) * 0.3;

    float ambient = 0.5;
    float lighting = ambient + keyDiffuse * 0.7 + fillDiffuse + rimDiffuse;
    lighting = min(lighting, 1.5);

    // Sample texture and apply lighting
    vec4 texColor = texture(uTexture, vTexCoord);
    oColor = vec4(texColor.rgb * lighting * vColor.rgb, texColor.a * vColor.a);
}
