#version 330

layout (location = 0) out vec4 fragColor;
layout (location = 1) out vec4 brightColor;

in vec2 fragTexCoord;

uniform vec3 u_emissiveColor;
uniform float u_emissiveIntensity;

uniform sampler2D emissionMap;

void main() {
    vec3 texColor = texture(emissionMap, fragTexCoord).rgb;
    float mask = dot(texColor, vec3(0.2126, 0.7152, 0.0722)); // Luminance mask
    vec3 emis = u_emissiveColor * u_emissiveIntensity * mask;
    fragColor = vec4(emis, 1.0);
    brightColor = vec4(emis, 1.0);
}
