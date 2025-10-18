#version 150

uniform mat4 ciModelViewProjection;

in vec4 ciPosition;     // Base line vertex (0 or 1)
in vec3 ciCustom0;      // Line start (CUSTOM_0)
in vec3 ciCustom1;      // Line end (CUSTOM_1)
in vec4 ciCustom2;      // Line color (CUSTOM_2)
in float ciCustom3;     // Line width (CUSTOM_3)

out vec4 vColor;

void main() {
    // Interpolate between start and end based on vertex position
    vec3 pos = mix(ciCustom0, ciCustom1, ciPosition.x);

    gl_Position = ciModelViewProjection * vec4(pos, 1.0);
    vColor = ciCustom2;
}
