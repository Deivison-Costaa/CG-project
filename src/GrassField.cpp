#include "GrassField.hpp"
#include <cstdlib>
#include <iostream>
#include "stb_image.h"

/**
 * @brief Construtor da classe GrassField.
 * @param terrain Referência ao objeto de terreno onde a grama será colocada.
 * @param shader Referência ao shader que será usado para renderizar a grama.
 * @param modelPath Caminho para o arquivo do modelo 3D da grama.
 * @param texturePath Caminho para o arquivo de textura da grama.
 * @param spacing O espaçamento entre cada possível tufo de grama.
 */
GrassField::GrassField(Terrain &terrain, Shader &shader, const std::string &modelPath, const std::string &texturePath, float spacing)
    : terrain(terrain), shader(shader), spacing(spacing), grassModel(modelPath, texturePath)
{
    setupInstancing();
}

/**
 * @brief Destrutor da classe GrassField.
 * Libera o buffer de VBO da instância para evitar vazamentos de memória na GPU.
 */
GrassField::~GrassField()
{
    if (!instanceMatrices.empty())
    {
        glDeleteBuffers(1, &instanceVBO);
    }
}

/**
 * @brief Configura e gera todas as instâncias de grama no cenário.
 * Esta função utiliza Ruído de Perlin para determinar a posição e a escala
 * de cada tufo de grama, resultando em uma distribuição natural.
 */
void GrassField::setupInstancing()
{
    // Parâmetros para a Geração Procedural da Grama

    // Define um limite máximo de instâncias para garantir a performance.
    const unsigned int maxGrassInstances = 10000;
    unsigned int currentInstanceCount = 0;

    // A frequência do ruído controla a aparência dos "aglomerados" de grama.
    // Valores maiores criam padrões menores e mais detalhados.
    // Valores menores criam padrões maiores e mais suaves, como grandes clareiras.
    float densityNoiseFrequency = 0.01f;
    float heightNoiseFrequency = 10.0f;

    // O limiar de densidade define quão "cheio" o campo de grama será.
    // O Ruído de Perlin gera valores entre -1.0 e 1.0. A grama só será
    // instanciada se o valor do ruído for maior que este limiar.
    // Aumente o valor para criar mais clareiras; diminua para um campo mais denso.
    float densityThreshold = 0.2f;

    // Itera sobre a grade do terreno para posicionar a grama.
    for (float x = 0; x < terrain.getWidth(); x += spacing)
    {
        for (float z = 0; z < terrain.getDepth(); z += spacing)
        {
            // Interrompe a geração se atingirmos o limite de instâncias.
            if (currentInstanceCount >= maxGrassInstances)
                break;

            // Lógica de Posicionamento por Altura
            // A grama só crescerá em faixas de altura específicas do terreno.
            float height = terrain.getHeight(x, z);
            float terrainAmplitude = 50.0f; // Deve ser o mesmo valor usado na geração do terreno.
            float height_normalized = (height / terrainAmplitude + 1.0f) / 2.0f;

            // Condição para que a grama cresça apenas em altitudes médias, evitando praias e picos de montanhas.
            if (height_normalized >= 0.4f && height_normalized < 0.7f)
            {
                // Lógica de Instanciação com Ruído de Perlin

                // 1. Calcula o ruído de densidade para a posição atual.
                // Convertemos as coordenadas para o espaço do mundo para que o padrão de ruído
                // seja consistente e não dependa do 'spacing'.
                float worldX = x - terrain.getWidth() / 2.0f;
                float worldZ = z - terrain.getDepth() / 2.0f;
                float densityNoise = db::perlin(worldX * densityNoiseFrequency, worldZ * densityNoiseFrequency);

                // 2. Verifica se o valor do ruído ultrapassa nosso limiar de densidade.
                if (densityNoise > densityThreshold)
                {
                    // Se sim, criamos uma instância de grama neste local.
                    // A matriz 'model' inicial apenas posiciona a grama no ponto correto.
                    glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(worldX, height, worldZ));

                    // 3. Utiliza um segundo ruído para variar a altura da grama.
                    // Isso evita que todos os tufos tenham o mesmo tamanho, adicionando realismo.
                    float heightNoise = db::perlin(worldX * heightNoiseFrequency, worldZ * heightNoiseFrequency);

                    // Mapeia o valor do ruído (de [-1, 1]) para um intervalo de escala desejado (ex: [0.005, 0.01]).
                    float minHeight = 0.000001f;
                    float maxHeight = 0.02f;
                    float height_scale = minHeight + (heightNoise + 1.0f) / 2.0f * (maxHeight - minHeight);

                    // A variação da largura é feita com um valor aleatório simples para maior variedade.
                    float width_scale = 0.009f + static_cast<float>(rand()) / RAND_MAX * 0.0005f;

                    // Aplica a escala à matriz do modelo e a armazena.
                    model = glm::scale(model, glm::vec3(width_scale, height_scale, width_scale));
                    instanceMatrices.push_back(model);
                    currentInstanceCount++;
                }
            }
        }
        if (currentInstanceCount >= maxGrassInstances)
            break;
    }

    std::cout << "Numero de tufos de grama gerados: " << instanceMatrices.size() << std::endl;

    // Se nenhuma instância foi gerada, não há necessidade de configurar os buffers.
    if (instanceMatrices.empty())
    {
        return;
    }

    // Configuração dos Buffers para Renderização Instanciada
    //  Enviamos todas as matrizes de modelo para a GPU de uma só vez.
    glGenBuffers(1, &instanceVBO);
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    glBufferData(GL_ARRAY_BUFFER, instanceMatrices.size() * sizeof(glm::mat4), &instanceMatrices[0], GL_STATIC_DRAW);

    // Agora, configuramos os atributos de vértice no VAO do modelo da grama.
    glBindVertexArray(grassModel.getVAO());

    // Uma matriz mat4 é, na verdade, composta por 4 vec4.
    // Precisamos configurar um atributo de vértice para cada uma dessas colunas.
    std::size_t vec4Size = sizeof(glm::vec4);
    for (unsigned int i = 0; i < 4; i++)
    {
        // Habilita os atributos de vértice a partir da localização 3 (0, 1, 2 já são usados para posição, normais, etc.).
        glEnableVertexAttribArray(3 + i);
        glVertexAttribPointer(3 + i, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void *)(i * vec4Size));

        // Informa ao OpenGL que este atributo deve ser atualizado por instância, não por vértice.
        glVertexAttribDivisor(3 + i, 1);
    }
    glBindVertexArray(0);
}

/**
 * @brief Desenha todas as instâncias de grama na tela.
 * @param view A matriz de visão da câmera.
 * @param projection A matriz de projeção da câmera.
 */
void GrassField::Draw(const glm::mat4 &view, const glm::mat4 &projection)
{
    // Não tenta desenhar se não houver grama para renderizar.
    if (instanceMatrices.empty())
    {
        return;
    }

    // Ativa e configura o shader da grama com as matrizes e texturas necessárias.
    shader.use();
    shader.setMat4("view", view);
    shader.setMat4("projection", projection);
    shader.setInt("texture_diffuse1", 0);

    // Ativa a unidade de textura e vincula a textura correta da grama.
    glActiveTexture(GL_TEXTURE0);
    grassModel.bindTexture();

    // Desenha todas as instâncias de grama com uma única chamada de renderização.
    glBindVertexArray(grassModel.getVAO());
    glDrawElementsInstanced(GL_TRIANGLES, grassModel.getIndicesCount(), GL_UNSIGNED_INT, 0, instanceMatrices.size());
    glBindVertexArray(0);
}