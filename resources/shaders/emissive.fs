#version 330

out vec4 fragColor;

in vec2 fragTexCoord;

uniform vec3 u_emissiveColor;
uniform float u_emissiveIntensity;

uniform sampler2D emissionMap;

void main() {
    vec3 texColor = texture(emissionMap, fragTexCoord).rgb;
    float mask = dot(texColor, vec3(0.2126, 0.7152, 0.0722)); // Luminance mask
    fragColor = vec4(u_emissiveColor * u_emissiveIntensity * mask, 1.0);
}
