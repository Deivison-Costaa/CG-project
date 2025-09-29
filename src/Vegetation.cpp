#include "Vegetation.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <cstdlib> // Para rand()
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

/**
 * @brief Construtor que gera as matrizes de transformação para cada instância.
 */
Vegetation::Vegetation(Terrain &terrain, Shader &shader, Model &model, int count, float minHeight, float maxHeight, float scale, glm::vec3 modelUp)
    : m_shader(shader), m_model(model), m_count(count)
{
    m_modelMatrices.reserve(m_count); // Pré-aloca memória para evitar realocações.
    int terrainWidth = terrain.getWidth();
    int terrainDepth = terrain.getDepth();

    // Loop para gerar a posição e rotação de cada instância.
    for (int i = 0; i < m_count; ++i)
    {
        // Gera uma posição aleatória na grade do terreno.
        int randX = rand() % terrainWidth;
        int randZ = rand() % terrainDepth;
        // Obtém a altura do terreno nessa posição.
        float height = terrain.getHeight(randX, randZ);

        // Coloca a vegetação apenas se estiver dentro da faixa de altura especificada.
        if (height >= minHeight && height <= maxHeight)
        {
            // Converte as coordenadas da grade para coordenadas do mundo.
            float worldX = static_cast<float>(randX) - terrainWidth / 2.0f;
            float worldZ = static_cast<float>(randZ) - terrainDepth / 2.0f;

            // LÓGICA DE ROTAÇÃO PARA ALINHAMENTO COM O TERRENO

            // 1. Obtém a normal da superfície do terreno, que indica a sua inclinação.
            glm::vec3 terrainNormal = terrain.getNormal(worldX, worldZ);

            // 2. Calcula a rotação necessária para alinhar o vetor "para cima" do modelo com a normal do terreno.
            // O uso de quaterniões (glm::quat) é mais robusto para cálculos de rotação 3D.
            glm::quat rotationQuat = glm::rotation(modelUp, terrainNormal);
            glm::mat4 rotationMatrix = glm::toMat4(rotationQuat);

            // 3. Adiciona uma rotação aleatória em torno do eixo Y para variar a orientação.
            float randomYaw = glm::radians((float)(rand() % 360));
            rotationMatrix = glm::rotate(rotationMatrix, randomYaw, modelUp);

            // 4. Constrói a matriz de modelo final, aplicando translação, rotação e escala.
            glm::mat4 modelMatrix = glm::mat4(1.0f);
            modelMatrix = glm::translate(modelMatrix, glm::vec3(worldX, height, worldZ)); // Posição
            modelMatrix = modelMatrix * rotationMatrix;                                   // Rotação (alinhamento + aleatória)
            modelMatrix = glm::scale(modelMatrix, glm::vec3(scale));                      // Escala

            m_modelMatrices.push_back(modelMatrix);
        }
    }
    // Atualiza a contagem para o número real de instâncias geradas.
    m_count = m_modelMatrices.size();

    // Configura os buffers da GPU apenas se alguma instância foi criada.
    if (m_count > 0)
    {
        setupBuffers();
    }
}

/**
 * @brief Destrutor que libera o VBO de instâncias.
 */
Vegetation::~Vegetation()
{
    if (m_count > 0)
    {
        glDeleteBuffers(1, &m_instanceVBO);
    }
}

/**
 * @brief Envia as matrizes de modelo para a GPU e configura os atributos de vértice.
 */
void Vegetation::setupBuffers()
{
    // Gera e preenche o VBO com os dados de todas as matrizes de modelo.
    glGenBuffers(1, &m_instanceVBO);
    glBindBuffer(GL_ARRAY_BUFFER, m_instanceVBO);
    glBufferData(GL_ARRAY_BUFFER, m_count * sizeof(glm::mat4), m_modelMatrices.data(), GL_STATIC_DRAW);

    // Vincula o VAO do modelo original para adicionar a configuração de instanciamento.
    unsigned int VAO = m_model.getVAO();
    glBindVertexArray(VAO);

    // Configura os atributos de vértice para a matriz de instância.
    // Uma mat4 é tratada como 4 atributos vec4 no shader.
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void *)0);
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void *)(sizeof(glm::vec4)));
    glEnableVertexAttribArray(5);
    glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void *)(2 * sizeof(glm::vec4)));
    glEnableVertexAttribArray(6);
    glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void *)(3 * sizeof(glm::vec4)));

    // Informa ao OpenGL que estes atributos devem ser atualizados uma vez por instância, não por vértice.
    glVertexAttribDivisor(3, 1);
    glVertexAttribDivisor(4, 1);
    glVertexAttribDivisor(5, 1);
    glVertexAttribDivisor(6, 1);

    glBindVertexArray(0);
}

/**
 * @brief Desenha todas as instâncias do modelo.
 */
void Vegetation::Draw(const glm::mat4 &view, const glm::mat4 &projection)
{
    if (m_count == 0)
        return;

    // Ativa e configura o shader com os uniformes necessários.
    m_shader.use();
    m_shader.setMat4("projection", projection);
    m_shader.setMat4("view", view);
    m_shader.setInt("texture_diffuse1", 0);

    // Ativa e vincula a textura do modelo.
    glActiveTexture(GL_TEXTURE0);
    m_model.bindTexture();

    // Vincula o VAO e faz a chamada de desenho instanciada.
    glBindVertexArray(m_model.getVAO());
    glDrawElementsInstanced(GL_TRIANGLES, m_model.getIndicesCount(), GL_UNSIGNED_INT, 0, m_count);
    glBindVertexArray(0);
}