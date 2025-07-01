#version 330 core

// Input texture coordinates
in vec2 fragTexCoord;

// Input vertex color (unused here but provided by default vertex shader)
out vec4 fragColor;

// Sampler for the HDR framebuffer
uniform sampler2D texture0;
// Tint/color multiplier
uniform vec4 colDiffuse;

// Framebuffer resolution (match your window size)
const vec2 size = vec2(1000.0, 650.0);
// Number of samples per axis (odd number recommended)
const float samples = 7.0;
// Glow size factor: lower = tighter glow, higher = bigger blur
const float quality = 0.5;

void main() {
    // Accumulate neighboring texels
    vec4 sum = vec4(0.0);
    vec2 sizeFactor = (1.0 / size) * quality;

    // Base color from the HDR buffer
    vec4 source = texture(texture0, fragTexCoord);

    int range = int((samples - 1.0) * 0.5);
    for (int x = -range; x <= range; x++) {
        for (int y = -range; y <= range; y++) {
            sum += texture(texture0, fragTexCoord + vec2(float(x), float(y)) * sizeFactor);
        }
    }

    // Average the samples, add original, then apply tint
    vec4 blurred = sum / (samples * samples);
    fragColor = (blurred + source) * colDiffuse;
}
