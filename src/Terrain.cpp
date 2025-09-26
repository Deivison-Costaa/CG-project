#include "Terrain.hpp"
#define DB_PERLIN_IMPL
#include "db_perlin.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/glm.hpp>
#include <iostream>

Terrain::Terrain(int width, int depth, Shader &shader)
    : m_width(width), m_depth(depth), m_shader(shader)
{
    std::vector<float> vertices;
    std::vector<unsigned int> indices;

    m_heights.resize(m_width * m_depth);

    for (int z = 0; z < m_depth; ++z)
    {
        for (int x = 0; x < m_width; ++x)
        {
            float height = calculateHeight(x, z);
            m_heights[z * m_width + x] = height;

            // Posição
            vertices.push_back((float)x);
            vertices.push_back(height);
            vertices.push_back((float)z);

            // Normal
            glm::vec3 normal = calculateNormal(x, z);
            vertices.push_back(normal.x);
            vertices.push_back(normal.y);
            vertices.push_back(normal.z);
        }
    }

    // --- Índices ---
    for (int z = 0; z < m_depth - 1; ++z)
    {
        for (int x = 0; x < m_width - 1; ++x)
        {
            int topLeft = (z * m_width) + x;
            int topRight = topLeft + 1;
            int bottomLeft = ((z + 1) * m_width) + x;
            int bottomRight = bottomLeft + 1;

            indices.push_back(topLeft);
            indices.push_back(bottomLeft);
            indices.push_back(topRight);
            indices.push_back(topRight);
            indices.push_back(bottomLeft);
            indices.push_back(bottomRight);
        }
    }

    m_indexCount = indices.size();
    setupTerrain(vertices, indices);
}

Terrain::~Terrain()
{
    glDeleteVertexArrays(1, &m_VAO);
    glDeleteBuffers(1, &m_VBO);
    glDeleteBuffers(1, &m_EBO);
}

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

    // Stride: x, y, z, nx, ny, nz
    size_t stride = 6 * sizeof(float);

    // Posição
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void *)0);

    // Normal
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void *)(3 * sizeof(float)));

    glBindVertexArray(0);
}

void Terrain::Draw(const glm::mat4 &view, const glm::mat4 &projection)
{
    m_shader.use();
    glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(-m_width / 2.0f, 0.0f, -m_depth / 2.0f));

    m_shader.setMat4("projection", projection);
    m_shader.setMat4("view", view);
    m_shader.setMat4("model", model);

    glBindVertexArray(m_VAO);
    glDrawElements(GL_TRIANGLES, m_indexCount, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

int Terrain::getWidth() const { return m_width; }
int Terrain::getDepth() const { return m_depth; }

float Terrain::getHeight(int x, int z) const
{
    if (x < 0)
        x = 0;
    if (x >= m_width)
        x = m_width - 1;
    if (z < 0)
        z = 0;
    if (z >= m_depth)
        z = m_depth - 1;
    return m_heights[z * m_width + x];
}

glm::vec3 Terrain::getColor(int x, int z) const
{
    float h = getHeight(x, z);
    if (h < 10.0f)
        return glm::vec3(0.0f, 0.0f, 0.5f); // água
    else if (h < 20.0f)
        return glm::vec3(0.8f, 0.7f, 0.5f); // areia
    else if (h < 50.0f)
        return glm::vec3(0.1f, 0.6f, 0.1f); // grama
    else
        return glm::vec3(1.0f, 1.0f, 1.0f); // neve
}

float Terrain::calculateHeight(float x, float z) const
{
    float amplitude = 70.0f;
    float frequency = 0.005f;
    int octaves = 6;
    float lacunarity = 4.0f;
    float persistence = 0.15f;

    float total = 0.0f;
    for (int i = 0; i < octaves; ++i)
    {
        total += db::perlin(x * frequency, z * frequency) * amplitude;
        amplitude *= persistence;
        frequency *= lacunarity;
    }
    return total;
}

glm::vec3 Terrain::calculateNormal(int x, int z) const
{
    float heightL = calculateHeight(x - 1, z);
    float heightR = calculateHeight(x + 1, z);
    float heightD = calculateHeight(x, z - 1);
    float heightU = calculateHeight(x, z + 1);

    glm::vec3 normal(heightL - heightR, 2.0f, heightD - heightU);
    return glm::normalize(normal);
}
