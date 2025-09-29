#version 460 core

//Atributos de Vértice
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in mat4 aInstanceMatrix; // Matriz de transformação da instância

//Saídas para o Fragment Shader
out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoords;

//Uniforms
uniform mat4 projection;
uniform mat4 view;
uniform vec4 plane; // Plano de corte para a água

void main()
{
    // Posiciona e orienta o vértice no mundo usando a matriz da instância
    vec4 worldPosition = aInstanceMatrix * vec4(aPos, 1.0);
    FragPos = worldPosition.xyz;
    Normal = normalize(mat3(transpose(inverse(aInstanceMatrix))) * aNormal);
    TexCoords = aTexCoords;

    // Aplica o plano de corte (para reflexo/refração da água)
    gl_ClipDistance[0] = dot(worldPosition, plane);

    // Posição final no ecrã
    gl_Position = projection * view * worldPosition;
}