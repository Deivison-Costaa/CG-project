#version 460 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in float Height;

// Propriedades do material (terreno)
uniform float terrainAmplitude;

// Propriedades da luz
uniform vec3 lightDir; // Direção da luz (já normalizada)
uniform vec3 lightColor;
uniform vec3 viewPos; // Posição da câmera

void main()
{
    // --- Cor base do terreno com base na altura ---
    float height_normalized = (Height / terrainAmplitude + 1.0) / 2.0;
    height_normalized = clamp(height_normalized, 0.0, 1.0);
    
    vec3 lowColor = vec3(0.54, 0.27, 0.07); // Marrom (rocha)
    vec3 midColor = vec3(0.0, 0.5, 0.1);    // Verde
    vec3 highColor = vec3(1.0, 1.0, 1.0);   // Branco (neve)

    vec3 terrainColor;
    if (height_normalized < 0.2) {
        terrainColor = lowColor;
    } else if (height_normalized < 0.65) {
        terrainColor = mix(lowColor, midColor, (height_normalized - 0.2) / 0.4);
    } else {
        terrainColor = mix(midColor, highColor, (height_normalized - 0.65) / 0.2);
    }

    // --- Iluminação ---
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