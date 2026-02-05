#version 150

uniform mat4 ciModelViewProjection;
uniform mat4 ciModelView;
uniform mat3 ciNormalMatrix;

in vec4 ciPosition;
in vec3 ciNormal;

// Instance attributes
in vec3 ciCustom0;    // Instance position
in vec4 ciCustom1;    // Instance color
in vec3 ciCustom2;    // Instance scale (using X component as radius)

out vec3 vReflectDir;
out float vLightIntensity;
out vec4 vColor;

uniform vec3 uLightPos;

void main() {
    // Apply per-instance scale and position
    vec3 scaledPosition = ciPosition.xyz * ciCustom2.x;  // Uniform sphere scale
    vec4 worldPosition = vec4(scaledPosition + ciCustom0, 1.0);

    gl_Position = ciModelViewProjection * worldPosition;

    // Calculate reflection direction for environment mapping
    vec3 normal = normalize(ciNormalMatrix * ciNormal);
    vec3 eyeDir = (ciModelView * worldPosition).xyz;
    vReflectDir = reflect(eyeDir, normal);

    // Simple lighting
    vLightIntensity = max(dot(normalize(uLightPos - eyeDir), normal), 0.0);

    // Pass instance color
    vColor = ciCustom1;
}
