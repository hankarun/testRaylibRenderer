#version 330 core

layout(location = 0) in vec3 vertexPosition;

uniform mat4 rotView;
uniform mat4 matProjection;

out vec3 texDir;

void main() {
    texDir = vertexPosition;
    gl_Position = matProjection * (rotView * vec4(vertexPosition, 1.0));
}
