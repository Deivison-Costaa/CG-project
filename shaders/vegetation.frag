#version 460 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

uniform sampler2D texture_diffuse1;

// Propriedades da luz (recebidas do C++)
uniform vec3 lightDir;
uniform vec3 lightColor;
uniform vec3 viewPos;

void main()
{
    // Amostra a cor da textura
    vec4 texColor = texture(texture_diffuse1, TexCoords);

    // Alpha testing: descarta fragmentos transparentes
    if(texColor.a < 0.1)
        discard;

    //Iluminação
    // Ambiente
    float ambientStrength = 0.4;
    vec3 ambient = ambientStrength * lightColor;

    // Difusa
    vec3 norm = normalize(Normal);
    vec3 lightDirectionToFrag = normalize(-lightDir);
    float diff = max(dot(norm, lightDirectionToFrag), 0.0);
    vec3 diffuse = diff * lightColor;

    // Especular (muito fraca para vegetação)
    float specularStrength = 0.1;
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 4);
    vec3 specular = specularStrength * spec * lightColor;

    // Combina a cor da textura com a iluminação
    vec3 result = (ambient + diffuse) * texColor.rgb + specular;

    // Define a cor final, mantendo a transparência da textura
    FragColor = vec4(result, texColor.a);
}