#version 150

uniform mat4 ciModelViewProjection;
uniform mat4 ciModelView;
uniform mat3 ciNormalMatrix;

in vec4 ciPosition;
in vec3 ciNormal;
in vec4 ciColor;

// Instance attributes (per-instance data)
in vec3 iPosition;    // Instance position
in vec4 iColor;       // Instance color
in vec3 iScale;       // Instance scale

out vec4 vColor;
out vec3 vNormal;
out vec3 vViewPosition;

void main() {
    // Apply per-instance scale and position
    vec4 scaledPosition = vec4(ciPosition.xyz * iScale, 1.0);
    vec4 worldPosition = scaledPosition + vec4(iPosition, 0.0);

    gl_Position = ciModelViewProjection * worldPosition;

    // Pass color to fragment shader
    vColor = iColor * ciColor;

    // Transform normal for lighting
    vNormal = ciNormalMatrix * ciNormal;

    // View space position for lighting
    vViewPosition = (ciModelView * worldPosition).xyz;
}
