#ifndef SUN_H
#define SUN_H

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include "Shader.hpp"

class Sun
{
public:
    // Construtor que recebe o shader para desenhar o sol
    Sun(Shader &shader);
    ~Sun();

    // Atualiza a posição e as cores do sol com base no tempo do jogo (0.0 a 1.0)
    void Update(float gameTime);

    // Desenha o objeto sol na cena
    void Draw(const glm::mat4 &view, const glm::mat4 &projection);

    // Funções "getter" para que o resto da cena possa obter informações de iluminação
    glm::vec3 GetLightDirection() const { return m_lightDirection; }
    glm::vec3 GetLightColor() const { return m_lightColor; }
    glm::vec3 GetSkyColor() const { return m_skyColor; }

private:
    // Dados do objeto
    Shader &m_shader;
    unsigned int m_vao, m_vbo, m_ebo;
    unsigned int m_indexCount;

    // Estado do sol
    glm::vec3 m_position;
    glm::vec3 m_lightDirection;
    glm::vec3 m_lightColor;
    glm::vec3 m_skyColor;

    // Função auxiliar para criar a malha da esfera
    void setupSphereMesh();
};

#endif // SUN_H