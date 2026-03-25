#version 150

in vec4 ciPosition;   // world-space (model matrix = identity)
in vec3 ciNormal;

uniform mat4 ciModelViewProjection;

out vec3 vWorldPos;
out vec3 vNormal;

void main() {
    vWorldPos   = ciPosition.xyz;
    vNormal     = ciNormal;
    gl_Position = ciModelViewProjection * ciPosition;
}
