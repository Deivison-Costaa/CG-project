#version 460 core
out vec4 FragColor;

// Entradas do Vertex Shader
in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;
in float Height;

// Texturas
uniform sampler2D grassTexture;
uniform sampler2D rockTexture;
uniform sampler2D sandTexture;

// Propriedades do material (terreno)
uniform float terrainAmplitude;

// Propriedades da luz
uniform vec3 lightDir;
uniform vec3 lightColor;
uniform vec3 viewPos;

void main()
{
    //Cor base a partir das texturas
    vec3 grassColor = texture(grassTexture, TexCoords * 20.0).rgb;
    vec3 rockColor = texture(rockTexture, TexCoords * 15.0).rgb;
    vec3 sandColor = texture(sandTexture, TexCoords * 15.0).rgb;

    //Lógica de mistura (Blend) em duas etapas
    float height_normalized = (Height / terrainAmplitude + 1.0) / 2.0;

    // Etapa 1: Misturar Areia e Grama nas altitudes baixas
    // Define a faixa onde a areia faz a transição para a grama (ex: entre 20% e 30% da altura)
    float sandToGrassFactor = smoothstep(0.20, 0.50, height_normalized);
    vec3 sandGrassColor = mix(sandColor, grassColor, sandToGrassFactor);

    // Etapa 2: Misturar o resultado (Areia/Grama) com a Rocha
    // Fator de rocha baseado na INCLINAÇÃO
    vec3 up = vec3(0.0, 1.0, 0.0);
    float slopeFactor = 1.0 - dot(normalize(Normal), up);
    slopeFactor = smoothstep(0.3, 0.6, slopeFactor); // Íngremes ficam mais rochosos

    // Fator de rocha baseado na ALTURA
    float rockHeightFactor = smoothstep(0.6, 0.8, height_normalized); // Alturas elevadas ficam mais rochosas

    // Combina os dois fatores, dando prioridade para a inclinação
    float finalRockFactor = clamp(rockHeightFactor + slopeFactor, 0.0, 1.0);

    // Mistura final: (Areia(agua)/Grama) com a Rocha
    vec3 terrainColor = mix(sandGrassColor, rockColor, finalRockFactor);

    //Iluminação (mesma lógica de antes)
    // Ambiente
    float ambientStrength = 0.3;
    vec3 ambient = ambientStrength * lightColor;

    // Difusa
    vec3 norm = normalize(Normal);
    float diff = max(dot(norm, -lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    // Especular
    float specularStrength = 0.2;
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * lightColor;

    vec3 result = (ambient + diffuse + specular) * terrainColor;
    FragColor = vec4(result, 1.0);
}