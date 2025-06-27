#version 330
out vec4 fragColor;
uniform vec2 u_resolution;
uniform vec2 u_mouse;
uniform float u_time;

void main() {
    vec4 color = vec4(1.0, 0.0, 0.0, 1.0);
    color.x = (sin(u_time) + 1.0) * 0.5;
    color.y = (cos(u_time * 1.1) + 1.0) * 0.5;
    color.z = (sin(u_time + 2.3) + 1.0) * 0.5;
    fragColor = color;
}

