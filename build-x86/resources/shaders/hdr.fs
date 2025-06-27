#version 330

out vec4 fragColor;
  
in vec2 fragTexCoord;

uniform float u_gamma;
uniform float u_exposure;
uniform sampler2D hdrBuffer;

void main() {             
    vec3 hdrColor = texture(hdrBuffer, fragTexCoord).rgb;

    vec3 mapped = vec3(1.0) - exp(-hdrColor * u_exposure);
    mapped = pow(mapped, vec3(1.0 / u_gamma));

    fragColor = vec4(mapped, 1.0);
}  
