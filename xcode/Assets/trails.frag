#version 150

uniform sampler2D uTexture;

in vec2 TexCoord;
out vec4 oColor;

void main() {
    // Simple passthrough - the trail effect is done by not clearing the FBO
    oColor = texture(uTexture, TexCoord);
}
