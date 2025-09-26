#version 460 core
layout (location = 0) in vec3 aPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

// NOVO: Passa a posição local (do modelo) para o fragment shader
out vec3 localPos;

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    localPos = aPos; // <-- Adicione esta linha
}