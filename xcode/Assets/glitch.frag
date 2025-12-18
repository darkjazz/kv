#version 150

uniform sampler2D uTexture;
uniform vec4 uParams;  // x=amount, y=time, z=rgbOffset, w=blockiness

in vec2 TexCoord;
out vec4 oColor;

// Hash function for pseudo-random numbers
float hash(vec2 p) {
    return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453);
}

// Glitch effect with RGB offset, scanlines, and block displacement
void main() {
    float amount = uParams.x;
    float time = uParams.y;
    float rgbOffset = uParams.z;
    float blockiness = uParams.w;

    if (amount <= 0.0) {
        oColor = texture(uTexture, TexCoord);
        return;
    }

    vec2 uv = TexCoord;

    // Horizontal line displacement
    float lineNoise = hash(vec2(floor(uv.y * 100.0), floor(time * 10.0)));
    if (lineNoise > 0.9) {
        uv.x += (hash(vec2(time, uv.y)) - 0.5) * amount * 0.1;
    }

    // Block displacement
    vec2 blockUV = floor(uv * vec2(20.0 * blockiness, 15.0 * blockiness));
    float blockNoise = hash(vec2(blockUV.x, blockUV.y + floor(time * 5.0)));
    if (blockNoise > 0.95) {
        uv.x += (hash(blockUV + time) - 0.5) * amount * 0.2;
    }

    // RGB channel offset (chromatic aberration)
    float offset = amount * rgbOffset * 0.01;
    float r = texture(uTexture, uv + vec2(offset, 0.0)).r;
    float g = texture(uTexture, uv).g;
    float b = texture(uTexture, uv - vec2(offset, 0.0)).b;

    oColor = vec4(r, g, b, 1.0);

    // Add scanlines
    float scanline = sin(uv.y * 800.0) * 0.04;
    oColor.rgb -= scanline * amount;

    // Random static noise
    float noise = hash(uv + time) * 0.05 * amount;
    oColor.rgb += noise;
}
