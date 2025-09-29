#ifndef SUN_H
#define SUN_H

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include "Shader.hpp"

/**
 * @class Sun
 * @brief Gerencia a lógica e a renderização do sol na cena.
 *
 * Esta classe é responsável por duas funções principais:
 * 1. Atuar como a principal fonte de luz direcional, calculando sua posição,
 * cor e a cor do céu com base em um ciclo de dia/noite.
 * 2. Renderizar uma representação visual do sol (uma esfera) no céu.
 */
class Sun
{
public:
    /**
     * @brief Construtor da classe Sun.
     * @param shader A referência ao programa de shader que será usado para desenhar o sol.
     */
    Sun(Shader &shader);
    ~Sun(); // Destrutor para liberar os recursos da GPU.

    /**
     * @brief Atualiza o estado do sol.
     * @param gameTime Um valor de 0.0 a 1.0 que representa o progresso no ciclo de 24h.
     */
    void Update(float gameTime);

    /**
     * @brief Desenha a esfera do sol na cena.
     * @param view A matriz de visão da câmera.
     * @param projection A matriz de projeção da câmera.
     */
    void Draw(const glm::mat4 &view, const glm::mat4 &projection);

    // Getters
    //  Fornecem as propriedades de iluminação calculadas para o resto da cena.
    glm::vec3 GetLightDirection() const { return m_lightDirection; }
    glm::vec3 GetLightColor() const { return m_lightColor; }
    glm::vec3 GetSkyColor() const { return m_skyColor; }

private:
    // Referência ao shader do sol.
    Shader &m_shader;
    // IDs dos objetos OpenGL para a malha da esfera.
    unsigned int m_vao, m_vbo, m_ebo;
    unsigned int m_indexCount; // Número de índices para desenhar a esfera.

    glm::vec3 m_position;       // Posição atual no espaço do mundo.
    glm::vec3 m_lightDirection; // Direção normalizada da luz.
    glm::vec3 m_lightColor;     // Cor da luz emitida.
    glm::vec3 m_skyColor;       // Cor de fundo do céu.

    /**
     * @brief Gera a geometria de uma esfera e a envia para a GPU.
     */
    void setupSphereMesh();
};

#endif