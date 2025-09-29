#version 460 core

out vec4 FragColor;

//Entradas do Vertex Shader
in vec4 clipSpace;
in vec2 textureCoords;
in vec3 toCameraVector;
in vec3 fromLightVector;

//Uniforms
uniform sampler2D reflectionTexture;  // Textura da cena refletida
uniform sampler2D refractionTexture;  // Textura da cena refratada
uniform sampler2D dudvMap;  // Mapa de distorção para simular ondas
uniform sampler2D normalMap; // Mapa de normais para iluminação detalhada
uniform float moveFactor;  // Fator de tempo para animar as ondas
uniform vec3 lightColor;

const float WAVE_STRENGTH = 0.2; // Controla a intensidade da distorção

void main()
{
    // Normaliza o vetor de visão
    vec3 viewVector = normalize(toCameraVector);

    // Converte a posição de clip-space para coordenadas de textura (NDC)
    vec2 ndc = (clipSpace.xy / clipSpace.w) * 0.5 + 0.5;
    vec2 refractTexCoords = ndc;
    vec2 reflectTexCoords = vec2(ndc.x, 1.0 - ndc.y); // Inverte Y para a reflexão

    // Distorce as coordenadas de textura usando o DUDV map para criar o efeito de ondas
    vec2 distortion = (texture(dudvMap, textureCoords + moveFactor).rg * 2.0 - 1.0) * WAVE_STRENGTH;
    refractTexCoords += distortion;
    reflectTexCoords += distortion;

    // Garante que as coordenadas distorcidas permaneçam dentro dos limites da textura
    refractTexCoords = clamp(refractTexCoords, 0.001, 0.999);
    reflectTexCoords = clamp(reflectTexCoords, 0.001, 0.999);

    // Amostra as cores da reflexão e da refração usando as coordenadas distorcidas
    vec4 reflectColor = texture(reflectionTexture, reflectTexCoords);
    vec4 refractColor = texture(refractionTexture, refractTexCoords);

    // Obtém a normal do normal map para iluminação detalhada
    vec3 normal = normalize(texture(normalMap, textureCoords).rgb * 2.0 - 1.0);

    // Calcula o fator Fresnel para misturar reflexão e refração
    // Ângulos rasos refletem mais, ângulos diretos refratam mais
    float fresnelFactor = pow(max(dot(viewVector, normal), 0.0), 1.5);

    // Mistura as duas cores com base no efeito Fresnel
    vec4 finalColor = mix(refractColor, reflectColor, fresnelFactor);
    
    // Adiciona o brilho especular do sol na superfície
    vec3 reflectedLight = reflect(normalize(fromLightVector), normal);
    float specular = pow(max(dot(reflectedLight, viewVector), 0.0), 100.0);

    // Adiciona o brilho especular à cor final
    FragColor = finalColor + vec4(lightColor * specular, 0.0);
    // Define a transparência final
    FragColor.a = clamp(fresnelFactor, 0.4, 1.0);
}