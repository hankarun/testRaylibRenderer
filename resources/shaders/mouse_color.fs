#version 330
out vec4 fragColor;
uniform vec2 u_resolution;
uniform vec2 u_mouse;
uniform float u_time;

void main() {
    vec2 uv = gl_FragCoord.xy / u_resolution;
    vec2 mouse = u_mouse / u_resolution;
    vec2 rg = abs(uv - mouse);
    fragColor = vec4(rg.x, rg.y, 0.0, 1.0);
}

