#version 150

uniform float uTime;
uniform vec3 uLightPos;

in vec4 vColor;
in vec3 vNormal;

out vec4 oColor;

void main() {
    vec3 normal = normalize(vNormal);
    
    // Dynamic lighting that moves over time
    vec3 lightDir = normalize(uLightPos + vec3(cos(uTime), sin(uTime * 0.7), sin(uTime * 0.5)) * 20.0);
    float diffuse = max(dot(normal, lightDir), 0.0);
    
    // Rim lighting for edge glow
    vec3 viewDir = vec3(0.0, 0.0, 1.0);
    float rim = 1.0 - max(dot(normal, viewDir), 0.0);
    rim = pow(rim, 3.0) * 0.5;
    
    // Combine lighting
    float lighting = 0.3 + diffuse * 0.7 + rim;
    
    // Add slight fresnel transparency at edges
    float alpha = vColor.a * (0.7 + diffuse * 0.3);
    
    oColor = vec4(vColor.rgb * lighting, alpha);
}
