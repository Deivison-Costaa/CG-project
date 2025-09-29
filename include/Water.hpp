#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include "Shader.hpp"

/**
 * @class Water
 * @brief Representa a superfície da água na cena.
 *
 * Esta classe é responsável unicamente por criar e desenhar uma malha plana (um quad).
 * Toda a lógica complexa para os efeitos visuais da água (reflexão, refração,
 * movimento de ondas) é gerenciada externamente, no loop de renderização principal
 * e nos shaders da água.
 */
class Water
{
public:
    /**
     * @brief Construtor da classe Water.
     * @param width A largura da superfície da água.
     * @param depth A profundidade da superfície da água.
     * @param shader A referência ao shader da água (embora não seja diretamente usada nesta classe, é uma boa prática passá-la).
     */
    Water(float width, float depth, Shader &shader);
    ~Water(); // Destrutor para liberar os recursos da GPU.

    /**
     * @brief Desenha a malha da superfície da água.
     * @param modelMatrix A matriz de modelo para posicionar a água na cena (geralmente define a altura Y).
     */
    void Draw(const glm::mat4 &modelMatrix);

private:
    /**
     * @brief Gera a geometria do plano e a envia para a GPU.
     */
    void setupMesh();

    // IDs dos objetos OpenGL para a malha.
    unsigned int VAO, VBO, EBO;
    // Dimensões da malha.
    float m_width, m_depth;
};