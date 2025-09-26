#include "Sun.hpp"
#include <vector>
#include <algorithm> // Para std::max
#include <glm/gtc/matrix_transform.hpp>

Sun::Sun(Shader &shader) : m_shader(shader)
{
    // Inicializa valores padrão
    m_position = glm::vec3(0.0f);
    m_lightDirection = glm::vec3(0.0f, -1.0f, 0.0f);
    m_lightColor = glm::vec3(1.0f);
    m_skyColor = glm::vec3(0.5f, 0.7f, 0.9f);

    // A lógica para criar a esfera agora pertence à classe Sun
    setupSphereMesh();
}

Sun::~Sun()
{
    // Limpa os buffers da GPU quando o objeto for destruído
    glDeleteVertexArrays(1, &m_vao);
    glDeleteBuffers(1, &m_vbo);
    glDeleteBuffers(1, &m_ebo);
}

void Sun::Update(float gameTime)
{
    // 1. Calcular a posição do Sol em um círculo
    float sunAngle = gameTime * 2.0f * glm::pi<float>();
    float orbitRadius = 400.0f;
    m_position.x = cos(sunAngle) * orbitRadius;
    m_position.y = sin(sunAngle) * orbitRadius;
    m_position.z = -150.0f;

    // 2. Calcular a direção da luz
    m_lightDirection = -glm::normalize(m_position);

    // 3. Calcular a cor da LUZ e do CÉU
    glm::vec3 middayColor(1.0f, 1.0f, 0.95f);
    glm::vec3 sunriseColor(1.0f, 0.6f, 0.2f);
    glm::vec3 nightColor(0.2f, 0.2f, 0.1f);

    glm::vec3 middaySky(0.5f, 0.7f, 0.9f);
    glm::vec3 sunriseSky(0.9f, 0.5f, 0.4f);
    glm::vec3 nightSky(0.01f, 0.01f, 0.05f);

    float sunHeightNormalized = std::max(0.0f, m_position.y / orbitRadius);

    if (m_position.y > 0.0f)
    { // Dia
        m_lightColor = glm::mix(sunriseColor, middayColor, sunHeightNormalized);
        m_skyColor = glm::mix(sunriseSky, middaySky, sunHeightNormalized);
    }
    else
    { // Noite
        m_lightColor = nightColor;
        m_skyColor = nightSky;
    }
}

void Sun::Draw(const glm::mat4 &view, const glm::mat4 &projection)
{
    // --- Shader ---
    m_shader.use();

    // Passa matrizes para o shader (para a esfera, se quiser desenhar)
    m_shader.setMat4("projection", projection);
    m_shader.setMat4("view", view);

    // --- Cálculo da posição do sol em UV ---
    // Transformação para coordenadas NDC
    glm::vec4 sunClip = projection * view * glm::vec4(m_position, 1.0f);
    sunClip /= sunClip.w;

    // De NDC (-1,1) para UV (0,1)
    glm::vec2 sunUV = glm::vec2(sunClip.x * 0.5f + 0.5f, sunClip.y * 0.5f + 0.5f);

    // --- Passa parâmetros para o shader ---
    m_shader.setVec3("sunColor", m_lightColor * 1.5f); // multiplicador para brilho
    m_shader.setVec2("sunScreenPos", sunUV);
    m_shader.setFloat("sunRadius", 0.03f);  // ajuste conforme a tela
    m_shader.setFloat("haloRadius", 0.12f); // halo externo

    // --- Desenhar esfera do sol (opcional) ---
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, m_position);
    model = glm::scale(model, glm::vec3(30.0f)); // escala da esfera
    m_shader.setMat4("model", model);

    glBindVertexArray(m_vao);
    glDrawElements(GL_TRIANGLE_STRIP, m_indexCount, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void Sun::setupSphereMesh()
{
    // Esta é a mesma lógica de `renderSphere` de antes,
    // agora como um método privado da classe.
    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);
    glGenBuffers(1, &m_ebo);

    std::vector<glm::vec3> positions;
    std::vector<unsigned int> indices;

    const unsigned int X_SEGMENTS = 64;
    const unsigned int Y_SEGMENTS = 64;
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

    bool oddRow = false;
    for (unsigned int y = 0; y < Y_SEGMENTS; ++y)
    {
        if (!oddRow)
        {
            for (unsigned int x = 0; x <= X_SEGMENTS; ++x)
            {
                indices.push_back(y * (X_SEGMENTS + 1) + x);
                indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
            }
        }
        else
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

    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(glm::vec3), &positions[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void *)0);
    glBindVertexArray(0);
}

unsigned int sunQuadVAO = 0, sunQuadVBO = 0;
void setupSunQuad()
{
    float quadVertices[] = {
        // pos x, y
        -1.0f,  1.0f,
        -1.0f, -1.0f,
         1.0f, -1.0f,

        -1.0f,  1.0f,
         1.0f, -1.0f,
         1.0f,  1.0f
    };

    glGenVertexArrays(1, &sunQuadVAO);
    glGenBuffers(1, &sunQuadVBO);

    glBindVertexArray(sunQuadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, sunQuadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glBindVertexArray(0);
}
