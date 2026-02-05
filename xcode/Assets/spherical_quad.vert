#version 150

uniform mat4 ciModelViewProjection;
uniform mat4 ciModelView;
uniform mat3 ciNormalMatrix;

in vec4 ciPosition;  // Vertex position (already in world space from CPU)
in vec3 ciNormal;    // Vertex normal
in vec4 ciColor;     // Vertex color

out vec4 vColor;
out vec3 vNormal;

void main() {
    // Position is already computed on CPU in spherical coordinates
    gl_Position = ciModelViewProjection * ciPosition;

    // Pass through normal and color
    vNormal = ciNormalMatrix * ciNormal;
    vColor = ciColor;
}
