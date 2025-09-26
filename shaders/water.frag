#version 460 core

in vec3 Normal; // ← aqui está o que estava faltando
in vec2 TexCoords;
in vec3 FragPos;

uniform vec3 cameraPos;

out vec4 FragColor;

void main()
{
    vec3 lightDir = normalize(vec3(0.5, 1.0, 0.3));
    float diff = max(dot(normalize(Normal), lightDir), 0.0);
    vec3 viewDir = normalize(cameraPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, normalize(Normal));
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);

    vec3 waterColor = vec3(0.0, 0.3, 0.5);
    vec3 finalColor = waterColor * diff + vec3(1.0) * spec * 0.3;
    FragColor = vec4(finalColor, 0.6);
}
