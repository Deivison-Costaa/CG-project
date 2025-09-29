#version 460 core
layout (location = 0) in vec3 aPos;

//Saídas para o Fragment Shader
out vec4 clipSpace;     // Posição no espaço de recorte para projeção de textura
out vec2 textureCoords; // Coordenadas para mapas de onda (DUDV, normal)
out vec3 toCameraVector;  // Vetor do fragmento para a câmera
out vec3 fromLightVector; // Vetor do fragmento para a luz

//Uniforms
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec3 cameraPos;
uniform vec3 lightDir;

const float TILING = 10.0; // Controla a repetição das texturas de onda

void main()
{
    // Posição do vértice no espaço do mundo
    vec4 worldPosition = model * vec4(aPos, 1.0);

    // Posição final no ecrã (espaço de recorte)
    clipSpace = projection * view * worldPosition;
    gl_Position = clipSpace;

    // Gera coordenadas de textura para os mapas de onda, com repetição (tiling)
    textureCoords = aPos.xz * 0.5 + 0.5 * TILING;

    // Calcula os vetores necessários para os cálculos de iluminação e Fresnel no fragment shader
    toCameraVector = cameraPos - worldPosition.xyz;
    fromLightVector = -lightDir;
}