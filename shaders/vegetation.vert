#version 460 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in mat4 aInstanceMatrix; // Matriz de instância

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoords;

uniform mat4 projection;
uniform mat4 view;
uniform vec4 plane; // Uniform para o plano de corte

void main()
{
    // Calcula a posição no mundo usando a matriz da instância
    vec4 worldPosition = aInstanceMatrix * vec4(aPos, 1.0);
    FragPos = worldPosition.xyz;

    // A normal também usa a matriz da instância
    Normal = normalize(mat3(transpose(inverse(aInstanceMatrix))) * aNormal);
    
    TexCoords = aTexCoords;
    
    // Aplica o plano de corte
    gl_ClipDistance[0] = dot(worldPosition, plane);
    
    gl_Position = projection * view * worldPosition;
}