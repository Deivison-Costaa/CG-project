#include "Terrain.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <algorithm>

// Define e implementa as bibliotecas de cabeçalho único.
#define DB_PERLIN_IMPL
#include "db_perlin.hpp"
#include "stb_image.h"

/**
 * @brief Construtor que orquestra toda a criação do terreno procedural.
 */
Terrain::Terrain(int width, int depth, Shader &shader, const std::string &sandTexturePath, const std::string &grassTexturePath, const std::string &rockTexturePath)
    : m_width(width), m_depth(depth), m_shader(shader)
{
    // 1. Carrega as texturas que serão usadas para dar aparência ao terreno.
    m_grassTextureID = loadTexture(grassTexturePath.c_str());
    m_rockTextureID = loadTexture(rockTexturePath.c_str());
    m_sandTextureID = loadTexture(sandTexturePath.c_str());

    // 2. Gera a geometria do terreno.
    std::vector<float> vertices;       // Armazenará os atributos de todos os vértices (pos, normal, texcoord).
    std::vector<unsigned int> indices; // Armazenará a ordem de desenho dos vértices.
    m_heights.resize(m_width * m_depth);

    // Itera sobre cada ponto da grade para gerar os vértices.
    for (int z = 0; z < m_depth; ++z)
    {
        for (int x = 0; x < m_width; ++x)
        {
            // Calcula a altura do vértice usando Ruído de Perlin.
            float height = calculateHeight(x, z);
            m_heights[z * m_width + x] = height; // Armazena a altura para consultas futuras.

            // Adiciona os atributos do vértice à lista.
            // Posição
            vertices.push_back((float)x);
            vertices.push_back(height);
            vertices.push_back((float)z);
            // Normal (essencial para a iluminação).
            glm::vec3 normal = calculateNormal(x, z);
            vertices.push_back(normal.x);
            vertices.push_back(normal.y);
            vertices.push_back(normal.z);
            // Coordenadas de Textura.
            vertices.push_back((float)x / (float)m_width);
            vertices.push_back((float)z / (float)m_depth);
        }
    }

    // Gera os índices para conectar os vértices e formar triângulos.
    for (int z = 0; z < m_depth - 1; ++z)
    {
        for (int x = 0; x < m_width - 1; ++x)
        {
            int topLeft = (z * m_width) + x;
            int topRight = topLeft + 1;
            int bottomLeft = ((z + 1) * m_width) + x;
            int bottomRight = bottomLeft + 1;
            // Cada quadrado da grade é formado por dois triângulos.
            indices.push_back(topLeft);
            indices.push_back(bottomLeft);
            indices.push_back(topRight);
            indices.push_back(topRight);
            indices.push_back(bottomLeft);
            indices.push_back(bottomRight);
        }
    }
    m_indexCount = indices.size();

    // 3. Envia a geometria gerada para a GPU.
    setupTerrain(vertices, indices);
}

/**
 * @brief Destrutor que libera os recursos da GPU.
 */
Terrain::~Terrain()
{
    glDeleteVertexArrays(1, &m_VAO);
    glDeleteBuffers(1, &m_VBO);
    glDeleteBuffers(1, &m_EBO);
    // Também libera as texturas.
    glDeleteTextures(1, &m_grassTextureID);
    glDeleteTextures(1, &m_rockTextureID);
    glDeleteTextures(1, &m_sandTextureID);
}

/**
 * @brief Desenha o terreno.
 */
void Terrain::Draw(const glm::mat4 &view, const glm::mat4 &projection)
{
    m_shader.use();

    // Cria e envia a matriz de modelo para posicionar o terreno no centro da cena.
    glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(-m_width / 2.0f, 0.0f, -m_depth / 2.0f));
    m_shader.setMat4("projection", projection);
    m_shader.setMat4("view", view);
    m_shader.setMat4("model", model);

    // Ativa e vincula as múltiplas texturas a diferentes unidades de textura.
    // O shader usará essas unidades para misturar as texturas.
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_grassTextureID);
    m_shader.setInt("grassTexture", 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m_rockTextureID);
    m_shader.setInt("rockTexture", 1);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, m_sandTextureID);
    m_shader.setInt("sandTexture", 2);

    // Desenha a malha do terreno.
    glBindVertexArray(m_VAO);
    glDrawElements(GL_TRIANGLES, m_indexCount, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    // Boa prática: reativa a unidade de textura 0.
    glActiveTexture(GL_TEXTURE0);
}

/**
 * @brief Configura os buffers OpenGL com os dados da malha.
 */
void Terrain::setupTerrain(const std::vector<float> &vertices, const std::vector<unsigned int> &indices)
{
    glGenVertexArrays(1, &m_VAO);
    glGenBuffers(1, &m_VBO);
    glGenBuffers(1, &m_EBO);

    glBindVertexArray(m_VAO);

    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    // Define o layout dos atributos de vértice no VBO.
    size_t stride = 8 * sizeof(float); // Posição(3) + Normal(3) + TexCoord(2)
    // Atributo de Posição (layout = 0)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void *)0);
    // Atributo de Normal (layout = 1)
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void *)(3 * sizeof(float)));
    // Atributo de Coordenadas de Textura (layout = 2)
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void *)(6 * sizeof(float)));

    glBindVertexArray(0);
}

/**
 * @brief Carrega uma textura de um arquivo de imagem.
 */
unsigned int Terrain::loadTexture(const char *path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format = GL_RGB;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}

// Implementação dos Getters e Helpers

int Terrain::getWidth() const { return m_width; }
int Terrain::getDepth() const { return m_depth; }

/**
 * @brief Retorna a altura do terreno em uma coordenada (x, z) da grade.
 * Inclui verificação de limites para evitar acesso fora do vetor.
 */
float Terrain::getHeight(int x, int z) const
{
    x = std::max(0, std::min(m_width - 1, x));
    z = std::max(0, std::min(m_depth - 1, z));
    return m_heights[z * m_width + x];
}

/**
 * @brief Calcula a altura procedural usando múltiplas oitavas de Ruído de Perlin.
 * A combinação de várias camadas de ruído cria uma aparência mais natural e detalhada.
 */
float Terrain::calculateHeight(float x, float z) const
{
    float amplitude = 70.0f;   // Altura máxima inicial das "montanhas".
    float frequency = 0.005f;  // "Zoom" do ruído. Valores menores criam montanhas mais largas.
    int octaves = 6;           // Número de camadas de detalhe.
    float lacunarity = 4.0f;   // Aumenta a frequência a cada oitava (mais detalhes).
    float persistence = 0.15f; // Reduz a amplitude a cada oitava (detalhes menores).

    float total = 0.0f;
    for (int i = 0; i < octaves; ++i)
    {
        total += db::perlin(x * frequency, z * frequency) * amplitude;
        amplitude *= persistence;
        frequency *= lacunarity;
    }
    return total;
}

/**
 * @brief Calcula a normal da superfície em um ponto (x, z) da grade.
 * A normal é calculada com base na diferença de altura dos pontos vizinhos.
 */
glm::vec3 Terrain::calculateNormal(int x, int z) const
{
    float heightL = calculateHeight(x - 1, z); // Esquerda
    float heightR = calculateHeight(x + 1, z); // Direita
    float heightD = calculateHeight(x, z - 1); // Abaixo
    float heightU = calculateHeight(x, z + 1); // Acima

    // O vetor normal é perpendicular ao plano da superfície.
    glm::vec3 normal(heightL - heightR, 2.0f, heightD - heightU);
    return glm::normalize(normal);
}

/**
 * @brief Retorna a normal do terreno em uma coordenada do espaço do mundo.
 * Converte as coordenadas do mundo para as coordenadas da grade antes de calcular.
 */
glm::vec3 Terrain::getNormal(float x, float z) const
{
    int gridX = static_cast<int>(x + m_width / 2.0f);
    int gridZ = static_cast<int>(z + m_depth / 2.0f);

    gridX = std::max(0, std::min(m_width - 1, gridX));
    gridZ = std::max(0, std::min(m_depth - 1, gridZ));

    return calculateNormal(gridX, gridZ);
}