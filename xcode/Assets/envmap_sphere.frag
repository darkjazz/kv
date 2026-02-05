#version 150

uniform samplerCube uEnvMap;
uniform vec4 uBaseColor;
uniform float uMixRatio;

in vec3 vReflectDir;
in float vLightIntensity;
in vec4 vColor;

out vec4 oColor;

void main() {
    // Sample environment map
    vec4 envColor = texture(uEnvMap, vReflectDir);

    // Mix with base color and lighting
    vec4 base = vLightIntensity * uBaseColor * vColor;
    vec4 finalColor = mix(envColor, base, uMixRatio);

    oColor = finalColor;
}
