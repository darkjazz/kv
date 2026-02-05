#version 150

uniform mat4 ciModelViewProjection;
uniform mat4 ciModelView;
uniform mat3 ciNormalMatrix;

in vec4 ciPosition;
in vec3 ciNormal;
in vec4 ciColor;

// Instance attributes (mapped to CUSTOM_0-2)
in vec3 ciCustom0;    // Instance position (CUSTOM_0)
in vec4 ciCustom1;    // Instance color (CUSTOM_1)
in vec3 ciCustom2;    // Instance scale (CUSTOM_2)

out vec4 vColor;
out vec3 vNormal;
out vec3 vViewPosition;

void main() {
    // Apply per-instance scale and position
    vec4 scaledPosition = vec4(ciPosition.xyz * ciCustom2, 1.0);
    vec4 worldPosition = scaledPosition + vec4(ciCustom0, 0.0);

    gl_Position = ciModelViewProjection * worldPosition;

    // Pass color to fragment shader
    vColor = ciCustom1 * ciColor;

    // Transform normal for lighting
    vNormal = ciNormalMatrix * ciNormal;

    // View space position for lighting
    vViewPosition = (ciModelView * worldPosition).xyz;
}
