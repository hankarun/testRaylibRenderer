#version 330
#define MAX_LIGHTS 8

// output
out vec4 fragColor;

// interpolated from vertex shader
in vec3 fragPos;
in vec2 fragTexCoord;
in mat3 TBN;

// scene uniforms
uniform vec3 u_lightPos[MAX_LIGHTS];
uniform vec3 u_lightColor[MAX_LIGHTS];
uniform float u_lightIntensity[MAX_LIGHTS];
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

    // accumulators
    vec3 ambientAccum = vec3(0.0);
    vec3 diffuseAccum = vec3(0.0);
    vec3 specularAccum = vec3(0.0);

    for (int i = 0; i < MAX_LIGHTS; i++) {
        vec3 L = normalize(u_lightPos[i] - fragPos);

        // inverse‐square attenuation
        float dist = length(L);
        float att = 1/(dist*dist);

        // ambient
        ambientAccum += u_ambientColor * texColor;

        // diffuse
        float d = max(dot(N, L), 0.0);
        diffuseAccum += texColor * u_lightColor[i] * u_lightIntensity[i] * d * att;

        // specular
        vec3 R = reflect(-L, N);
        float s = pow(max(dot(R, V), 0.0), u_shininess);
        specularAccum += s * u_specularColor * u_lightColor[i] * u_lightIntensity[i] * att;
    }

    fragColor = vec4(ambientAccum + diffuseAccum + specularAccum, 1.0);
}


