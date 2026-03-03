#version 330 core

in vec3 vWorldPos;
in vec3 vWorldNormal;

out vec4 FragColor;

uniform vec3 uBaseColor;

uniform vec3 uLightDir;    // “光が進む方向”(directional). 例: normalize(vec3(-1,-1,-1))
uniform vec3 uLightColor;  // 例: vec3(1)
uniform float uAmbient;    // 例: 0.15

uniform vec3 uViewPos;     // カメラ位置（ワールド）
#if 1
void main()
{
    vec3 N = normalize(vWorldNormal);

    // directional light: 入射方向は -uLightDir と考えると扱いやすい
    vec3 L = normalize(-uLightDir);

    // Lambert diffuse
    float ndotl = max(dot(N, L), 0.0);
    vec3 diffuse = ndotl * uLightColor;

    // Simple specular (Blinn-Phong)
    vec3 V = normalize(uViewPos - vWorldPos);
    vec3 H = normalize(L + V);
    float specPow = 64.0;
    float spec = pow(max(dot(N, H), 0.0), specPow);
    vec3 specular = spec * uLightColor * 0.25;

    vec3 color = uBaseColor * (uAmbient + diffuse) + specular;
    FragColor = vec4(color, 1.0);
}
#else
void main()
{
    FragColor = vec4(1.0, 0.0, 1.0, 1.0); // ド派手マゼンタ
}
#endif