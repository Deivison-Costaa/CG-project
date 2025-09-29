#version 460 core

out vec4 FragColor;

//Entradas do Vertex Shader
in vec3 Normal;
in vec3 FragPos;
in vec2 TexCoords;

//Uniforms
uniform sampler2D texture_diffuse1;
uniform vec3 lightDir;
uniform vec3 lightColor;
uniform vec3 viewPos;

void main()
{
    // Obtém a cor da textura
    vec4 texColor = texture(texture_diffuse1, TexCoords);

    // Alpha-testing: descarta píxeis transparentes para recortar a forma
    if(texColor.a < 0.1)
        discard;

    //Iluminação de Phong
    // Ambiente
    vec3 ambient = 0.4 * lightColor;

    // Difusa
    vec3 norm = normalize(Normal);
    vec3 lightDirectionToFrag = normalize(-lightDir);
    float diff = max(dot(norm, lightDirectionToFrag), 0.0);
    vec3 diffuse = diff * lightColor;

    // Especular (brilho fraco para vegetação)
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 8);
    vec3 specular = 0.05 * spec * lightColor;

    // Combina a iluminação com a cor da textura
    vec3 result = (ambient + diffuse) * texColor.rgb + specular;

    // Cor final, preservando a transparência da textura
    FragColor = vec4(result, texColor.a);
}