#version 460 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out vec3 Normal;
out vec2 TexCoords;
out vec3 FragPos;
out float Height;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec4 plane; // Uniform para o plano de corte

void main()
{
    // Calcula a posição no mundo
    vec4 worldPosition = model * vec4(aPos, 1.0);
    FragPos = worldPosition.xyz;

    Normal = mat3(transpose(inverse(model))) * aNormal;
    TexCoords = aTexCoords;
    Height = aPos.y;
    
    // Aplica o plano de corte
    gl_ClipDistance[0] = dot(worldPosition, plane);
    
    gl_Position = projection * view * worldPosition;
}