#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;

uniform mat4 uModel;
uniform mat4 uViewProj;

out vec3 vWorldPos;
out vec3 vWorldNormal;

void main()
{
    vec4 worldPos = uModel * vec4(aPos, 1.0);
    vWorldPos = worldPos.xyz;

    // 正しい法線変換：inverse-transpose(model)
    mat3 normalMat = mat3(transpose(inverse(uModel)));
    vWorldNormal = normalize(normalMat * aNormal);

    gl_Position = uViewProj * worldPos;
}
