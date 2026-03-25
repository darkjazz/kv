#version 150

in vec2 vOldPos;   // original XZ water surface position
in vec2 vNewPos;   // refracted XZ floor position

uniform float uCausticScale;   // brightness multiplier (default ~1.5)

out vec4 oColor;

void main() {
    // Area of each "light bundle" triangle before and after refraction,
    // computed from screen-space derivatives of interpolated positions.
    //
    // oldArea: how much XZ area each mesh cell covers on the water surface.
    // newArea: how much XZ area that same cell covers after refraction on floor.
    // Brightness = oldArea / newArea  (focusing → bright, spreading → dim).

    vec2 dOldX = dFdx(vOldPos);
    vec2 dOldY = dFdy(vOldPos);
    vec2 dNewX = dFdx(vNewPos);
    vec2 dNewY = dFdy(vNewPos);

    float oldArea = abs(dOldX.x * dOldY.y - dOldX.y * dOldY.x);
    float newArea = abs(dNewX.x * dNewY.y - dNewX.y * dNewY.x);

    float brightness = 0.0;
    if (newArea > 1e-7) {
        brightness = (oldArea / newArea) * uCausticScale;
    }

    // Subtle colour: warm white-gold caustic light
    vec3 causticColor = vec3(1.0, 0.97, 0.85) * brightness;

    // Additive blend — output alpha = 0 to work with GL_ONE, GL_ONE
    oColor = vec4(causticColor, 1.0);
}
