#version 330 core

layout (location = 0) out vec4 fragColor;
layout (location = 1) out vec4 brightColor;

in vec3 texDir;

uniform samplerCube cubemap;

void main() {
    fragColor = texture(cubemap, normalize(texDir));
    fragColor = pow(fragColor, vec4(2.2));
    brightColor = vec4(0.0, 0.0, 0.0, 0.0);
}
