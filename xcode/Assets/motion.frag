#version 150

uniform sampler2D uTexture;
uniform vec4 uParams;  // x=angle(degrees), y=amount, z=samples, w=unused

in vec2 TexCoord;
out vec4 oColor;

// Directional motion blur
void main() {
    float angle = radians(uParams.x);
    float amount = uParams.y;
    int samples = int(uParams.z);

    if (amount <= 0.0 || samples <= 0) {
        oColor = texture(uTexture, TexCoord);
        return;
    }

    // Calculate direction vector from angle
    vec2 dir = vec2(cos(angle), sin(angle));

    vec4 color = vec4(0.0);

    // Sample along motion direction
    for (int i = 0; i < samples; i++) {
        float t = (float(i) / float(samples - 1)) - 0.5;  // -0.5 to 0.5, center the blur
        vec2 offset = dir * t * amount * 0.05;  // 5% of screen per unit
        color += texture(uTexture, TexCoord + offset);
    }

    oColor = color / float(samples);
}
