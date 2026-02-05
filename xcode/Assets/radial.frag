#version 150

uniform sampler2D uTexture;
uniform vec4 uParams;  // x=centerX, y=centerY, z=amount, w=samples

in vec2 TexCoord;
out vec4 oColor;

// Radial/zoom blur from center point
void main() {
    vec2 center = vec2(uParams.x, uParams.y);
    float amount = uParams.z;
    int samples = int(uParams.w);

    if (amount <= 0.0 || samples <= 0) {
        oColor = texture(uTexture, TexCoord);
        return;
    }

    vec2 dir = TexCoord - center;
    float dist = length(dir);
    vec4 color = vec4(0.0);

    // Sample along ray from center outward
    for (int i = 0; i < samples; i++) {
        float t = float(i) / float(samples - 1);  // 0.0 to 1.0
        vec2 samplePos = center + dir * (1.0 + (t - 0.5) * amount);
        color += texture(uTexture, samplePos);
    }

    oColor = color / float(samples);
}
