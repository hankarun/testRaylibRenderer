#version 330 core

out vec4 fragColor;
in vec3 texDir;

uniform samplerCube cubemap;

void main() {
    fragColor = texture(cubemap, normalize(texDir));
}
