#version 460 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal; // Novo atributo: normal

// Saídas para o Fragment Shader
out vec3 FragPos;
out vec3 Normal;
out float Height;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    FragPos = vec3(model * vec4(aPos, 1.0));
    Normal = mat3(transpose(inverse(model))) * aNormal; // Transforma a normal para world space
    Height = aPos.y;

    gl_Position = projection * view * vec4(FragPos, 1.0);
}