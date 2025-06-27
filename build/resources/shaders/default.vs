#version 330 core

layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec2 vertexTexCoord;
layout(location = 2) in vec3 vertexNormal;
layout(location = 3) in vec4 vertexTangent;
 
uniform mat4 matModel;
uniform mat4 matView;
uniform mat4 matProjection;

out vec3 fragPos;
out vec2 fragTexCoord;
out mat3 TBN;

void main() {
    // World-space position
    vec4 worldPos = matModel * vec4(vertexPosition, 1.0);
    fragPos = worldPos.xyz;

    // Build normal matrix
    mat3 normalMatrix = transpose(inverse(mat3(matModel)));
    vec3 worldNormal = normalize(normalMatrix * vertexNormal);

    // Derive a tangent & bitangent
    vec3 T = normalize(normalMatrix * vertexTangent.xyz);
    vec3 B = normalize(cross(worldNormal, T) * vertexTangent.w);

    // Pack them into TBN matrix
    TBN = mat3(T, B, worldNormal);

    // Pass UVs straight through 
    fragTexCoord = vertexTexCoord;

    // Final clip position
    gl_Position = matProjection * matView * worldPos;
}
