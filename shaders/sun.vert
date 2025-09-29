#version 460 core
layout (location = 0) in vec3 aPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec4 plane; // Uniform para o plano de corte

void main()
{
    // Calcula a posição no mundo
    vec4 worldPosition = model * vec4(aPos, 1.0);
    
    // Aplica o plano de corte
    gl_ClipDistance[0] = dot(worldPosition, plane);
    
    gl_Position = projection * view * worldPosition;
}