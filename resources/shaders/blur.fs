#version 330

out vec4 fragColor;

in vec2 fragTexCoord;

uniform sampler2D image;
uniform int u_horizontal;

const float weight[5] = float[] (0.227027, 0.1945946, 0.1216216, 0.05405405, 0.01621622);

void main() {
    vec2 texOffset = 1.0 / textureSize(image, 0); // gets size of single texel
    vec3 result = texture(image, fragTexCoord).rgb * weight[0]; // center pixel

    if (u_horizontal == 1) {
        for (int i = 1; i < 5; i++) {
            result += texture(image, fragTexCoord + vec2(texOffset.x * i, 0.0)).rgb * weight[i];
            result += texture(image, fragTexCoord - vec2(texOffset.x * i, 0.0)).rgb * weight[i];
        }
    } 
    else {
        for (int i = 1; i < 5; i++) {
            result += texture(image, fragTexCoord + vec2(0.0, texOffset.y * i)).rgb * weight[i];
            result += texture(image, fragTexCoord - vec2(0.0, texOffset.y * i)).rgb * weight[i];
        }
    }

    fragColor = vec4(result, 1.0);
}