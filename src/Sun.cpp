#include "Sun.hpp"
#include <vector>
#include <algorithm> // Para std::max
#include <glm/gtc/matrix_transform.hpp>

/**
 * @brief Construtor que inicializa o estado padrão do sol e gera sua malha.
 */
Sun::Sun(Shader &shader) : m_shader(shader)
{
    // Inicializa os valores padrão para garantir um estado válido antes da primeira atualização.
    m_position = glm::vec3(0.0f);
    m_lightDirection = glm::vec3(0.0f, -1.0f, 0.0f);
    m_lightColor = glm::vec3(1.0f);
    m_skyColor = glm::vec3(0.5f, 0.7f, 0.9f);

    // Gera a geometria da esfera que representa o sol.
    setupSphereMesh();
}

/**
 * @brief Destrutor que libera os buffers da GPU.
 */
Sun::~Sun()
{
    glDeleteVertexArrays(1, &m_vao);
    glDeleteBuffers(1, &m_vbo);
    glDeleteBuffers(1, &m_ebo);
}

/**
 * @brief Atualiza a posição e as cores do sol para simular o ciclo dia/noite.
 */
void Sun::Update(float sunTime)
{
    // 1. Calcula a posição do sol em uma órbita circular.
    // 'sunTime' (0.0 a 1.0) é mapeado para um ângulo de 0 a 2*PI.
    float sunAngle = sunTime * 2.0f * glm::pi<float>();
    float orbitRadius = 400.0f;
    m_position.x = cos(sunAngle) * orbitRadius;
    m_position.y = sin(sunAngle) * orbitRadius; // O movimento no eixo Y simula o nascer e o pôr do sol.
    m_position.z = -150.0f;                     // Mantém o sol a uma distância fixa no eixo Z.

    // 2. Calcula a direção da luz.
    // É o vetor normalizado que aponta da posição do sol para a origem (0,0,0).
    m_lightDirection = -glm::normalize(m_position);

    // 3. Interpola as cores da LUZ e do CÉU com base na altura do sol.
    glm::vec3 middayColor(1.0f, 1.0f, 0.95f); // Branco/amarelado
    glm::vec3 sunriseColor(1.0f, 0.6f, 0.2f); // Laranja
    glm::vec3 nightColor(0.2f, 0.2f, 0.1f);   // Luz fraca

    glm::vec3 middaySky(0.5f, 0.7f, 0.9f);   // Azul claro
    glm::vec3 sunriseSky(0.9f, 0.5f, 0.4f);  // Céu alaranjado
    glm::vec3 nightSky(0.01f, 0.01f, 0.05f); // Céu escuro

    // Normaliza a altura do sol para um intervalo de 0.0 (horizonte) a 1.0 (pico).
    float sunHeightNormalized = std::max(0.0f, m_position.y / orbitRadius);

    if (m_position.y > 0.0f) // É dia
    {
        // Interpola linearmente entre a cor do nascer do sol e a cor do meio-dia.
        m_lightColor = glm::mix(sunriseColor, middayColor, sunHeightNormalized);
        m_skyColor = glm::mix(sunriseSky, middaySky, sunHeightNormalized);
    }
    else // É noite
    {
        // Usa cores fixas para a noite.
        m_lightColor = nightColor;
        m_skyColor = nightSky;
    }
}

/**
 * @brief Desenha a representação visual do sol.
 */
void Sun::Draw(const glm::mat4 &view, const glm::mat4 &projection)
{
    m_shader.use();

    // Envia as matrizes de câmera para o shader, para que a esfera seja posicionada corretamente.
    m_shader.setMat4("projection", projection);
    m_shader.setMat4("view", view);

    // (Esta seção de código foi removida da sua versão, mas estava na versão anterior do projeto)
    // Cálculo da posição do sol em coordenadas de tela (UV)
    // Isto é útil se o shader do sol desenha um brilho (halo) como um efeito de pós-processamento.
    // glm::vec4 sunClip = projection * view * glm::vec4(m_position, 1.0f);
    // sunClip /= sunClip.w;
    // glm::vec2 sunUV = glm::vec2(sunClip.x * 0.5f + 0.5f, sunClip.y * 0.5f + 0.5f);
    // m_shader.setVec2("sunScreenPos", sunUV);

    // uniforms para o shader do sol
    m_shader.setVec3("sunColor", m_lightColor * 1.5f); // Multiplicador para dar um brilho extra.

    // Constrói a matriz de modelo para a esfera do sol (posição e escala).
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, m_position);
    model = glm::scale(model, glm::vec3(30.0f)); // Torna a esfera grande o suficiente para ser visível.
    m_shader.setMat4("model", model);

    // Desenha a malha da esfera.
    glBindVertexArray(m_vao);
    glDrawElements(GL_TRIANGLE_STRIP, m_indexCount, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

/**
 * @brief Gera proceduralmente a malha de uma esfera.
 * Cria os vértices e índices para uma esfera UV e os envia para a GPU.
 */
void Sun::setupSphereMesh()
{
    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);
    glGenBuffers(1, &m_ebo);

    std::vector<glm::vec3> positions;
    std::vector<unsigned int> indices;

    const unsigned int X_SEGMENTS = 64;
    const unsigned int Y_SEGMENTS = 64;

    // Gera os vértices da esfera usando coordenadas esféricas.
    for (unsigned int y = 0; y <= Y_SEGMENTS; ++y)
    {
        for (unsigned int x = 0; x <= X_SEGMENTS; ++x)
        {
            float xSegment = (float)x / (float)X_SEGMENTS;
            float ySegment = (float)y / (float)Y_SEGMENTS;
            float xPos = std::cos(xSegment * 2.0f * glm::pi<float>()) * std::sin(ySegment * glm::pi<float>());
            float yPos = std::cos(ySegment * glm::pi<float>());
            float zPos = std::sin(xSegment * 2.0f * glm::pi<float>()) * std::sin(ySegment * glm::pi<float>());
            positions.push_back(glm::vec3(xPos, yPos, zPos));
        }
    }

    // Gera os índices para formar uma 'triangle strip', que é uma forma eficiente de desenhar.
    bool oddRow = false;
    for (unsigned int y = 0; y < Y_SEGMENTS; ++y)
    {
        if (!oddRow) // Linha par: da esquerda para a direita.
        {
            for (unsigned int x = 0; x <= X_SEGMENTS; ++x)
            {
                indices.push_back(y * (X_SEGMENTS + 1) + x);
                indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
            }
        }
        else // Linha ímpar: da direita para a esquerda, para conectar a strip.
        {
            for (int x = X_SEGMENTS; x >= 0; --x)
            {
                indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
                indices.push_back(y * (X_SEGMENTS + 1) + x);
            }
        }
        oddRow = !oddRow;
    }
    m_indexCount = indices.size();

    // Envia os dados para a GPU.
    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(glm::vec3), &positions[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

    // Configura o atributo de vértice para a posição.
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void *)0);
    glBindVertexArray(0);
}