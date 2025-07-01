#version 330
#define MAX_LIGHTS 8

// outputs
layout (location = 0) out vec4 fragColor;
layout (location = 1) out vec4 brightColor;

// interpolated from vertex shader
in vec3 fragPos;
in vec2 fragTexCoord;
in mat3 TBN;

// light struct
struct Light {
    vec3 position;
    vec3 color;
    float intensity;
};

// scene uniforms
uniform Light u_lights[MAX_LIGHTS];
uniform vec3 u_eyePos;

// material uniforms
uniform vec3 u_ambientColor;
uniform vec3 u_specularColor;
uniform float u_shininess;

// moon texture
uniform sampler2D diffuseMap;
uniform sampler2D normalMap;

void main() {
    // compute bumped normal
    vec3 mapNormal = texture(normalMap, fragTexCoord).rgb * 2.0 - 1.0;

    // normalize inputs
    vec3 N = normalize(TBN * mapNormal);
    vec3 V = normalize(u_eyePos - fragPos);

    // texture
    vec3 texColor = texture(diffuseMap, fragTexCoord).rgb;

    texColor = pow(texColor, vec3(2.2)); // gamma correction

    // accumulators
    vec3 ambientAccum = vec3(0.0);
    vec3 diffuseAccum = vec3(0.0);
    vec3 specularAccum = vec3(0.0);

    for (int i = 0; i < MAX_LIGHTS; i++) {
        vec3 L = u_lights[i].position - fragPos;

        // inverse‐square attenuation
        float dist = length(L);
        float att = 1/max(1e-4f, dist*dist*dist);
        L = normalize(L);

        // ambient
        ambientAccum += u_ambientColor * texColor;

        // diffuse
        float d = max(dot(N, L), 0.0);
        diffuseAccum += texColor * u_lights[i].color * u_lights[i].intensity * d * att;

        // specular
        vec3 R = reflect(-L, N);
        float s = pow(max(dot(R, V), 0.0), u_shininess);
        specularAccum += s * u_specularColor * u_lights[i].color * u_lights[i].intensity * d * att;
    }

    fragColor = vec4(ambientAccum + diffuseAccum + specularAccum, 1.0);

    float brightness = dot(fragColor.rgb, vec3(0.2126, 0.7152, 0.0722));
    if (brightness > 2.5) 
        brightColor = vec4(fragColor.rgb, 1.0);
    else 
        brightColor = vec4(0.0, 0.0, 0.0, 1.0);
}


