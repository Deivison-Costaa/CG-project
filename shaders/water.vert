#version 460 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoords;

out vec2 TexCoords;
out vec3 FragPos;
out vec3 Normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform float time;

void main()
{
    vec3 pos = aPos;

    // Ondulações múltiplas
    float wave1 = sin(pos.x * 0.1 + time) * 1.0;
    float wave2 = cos(pos.z * 0.15 + time * 1.2) * 0.6;
    float wave3 = sin((pos.x + pos.z) * 0.2 + time * 0.7) * 0.4;
    pos.y = -15 + wave1 + wave2 + wave3;

    float wave4 = sin(pos.x * 2.0 + time * 2.5) * 0.05;
    float wave5 = cos(pos.z * 2.5 + time * 2.0) * 0.05;
    pos.y += wave4 + wave5;


    // Derivadas manuais
    float dYdx = 0.1 * cos(pos.x * 0.1 + time) * 1.0 + 0.2 * cos((pos.x + pos.z) * 0.2 + time * 0.7) * 0.4;
    float dYdz = -0.15 * sin(pos.z * 0.15 + time * 1.2) * 0.6 + 0.2 * cos((pos.x + pos.z) * 0.2 + time * 0.7) * 0.4;

    // Normal aproximada
    vec3 normal = normalize(vec3(-dYdx, 1.0, -dYdz));
    Normal = normalize(mat3(transpose(inverse(model))) * normal);

    FragPos = vec3(model * vec4(pos, 1.0));
    TexCoords = aTexCoords;

    gl_Position = projection * view * model * vec4(pos, 1.0);
}
