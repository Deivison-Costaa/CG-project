#include "Water.hpp"
#include <vector>

/**
 * @brief O construtor apenas inicializa a malha da água.
 */
Water::Water(float width, float depth, Shader &shader) : m_width(width), m_depth(depth)
{
    setupMesh();
}

/**
 * @brief Destrutor que libera os buffers da GPU.
 */
Water::~Water()
{
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
}

/**
 * @brief Cria e configura a geometria de um plano simples.
 * Esta malha servirá como a superfície sobre a qual os shaders desenharão os efeitos da água.
 */
void Water::setupMesh()
{
    float halfW = m_width / 2.0f;
    float halfD = m_depth / 2.0f;

    // A malha precisa apenas das posições dos vértices.
    // Todos os outros cálculos (normais, texcoords, etc.) são feitos no shader.
    float vertices[] = {
        // Posição (x, y, z)
        -halfW, 0.0f, -halfD,
        halfW, 0.0f, -halfD,
        halfW, 0.0f, halfD,
        -halfW, 0.0f, halfD};

    // Índices para desenhar o plano usando dois triângulos.
    unsigned int indices[] = {0, 1, 2, 2, 3, 0};

    // Configuração padrão dos buffers (VAO, VBO, EBO).
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Configura o atributo de vértice para a posição (layout = 0).
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
}

/**
 * @brief Desenha a malha da água.
 * Esta função é intencionalmente simples. Ela apenas emite o comando de desenho.
 */
void Water::Draw(const glm::mat4 &modelMatrix)
{
    // Nota de design: A ativação do shader da água e o envio de todos os uniforms
    // (texturas de reflexão/refração, mapas DUDV/normal, etc.)
    // são responsabilidade do chamador (neste caso, o loop principal em main.cpp).
    // Isso mantém a classe Water focada apenas na sua geometria.

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0); // Desenha os 2 triângulos (6 vértices).
    glBindVertexArray(0);
}