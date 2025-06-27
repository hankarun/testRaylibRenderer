#version 330
out vec4 fragColor;
uniform vec2 u_resolution;
uniform vec2 u_mouse;
uniform float u_time;

float plot(vec2 uv, float pct) {
    return smoothstep(0.98, 1.00, 1 - abs(uv.y - pow(uv.x, 3.0)));
}

void main() {
    vec2 uv = gl_FragCoord.xy / u_resolution;
    float y = pow(uv.x, 3.0);
    vec3 color = vec3(y);

    float pct = plot(uv, y);
    color = (1.0 - pct) * color + pct * vec3(1.0, 0.0, 0.0);

    fragColor = vec4(color, 1.0);
}

