#include "GrassField.hpp"
#include <cstdlib>

GrassField::GrassField(Terrain &terrain, Shader &shader, float spacing)
    : terrain(terrain), shader(shader), spacing(spacing)
{
    setupMesh();
    setupInstancing();
}

GrassField::~GrassField()
{
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &instanceVBO);
}

// Um triângulo simples como grama
void GrassField::setupMesh()
{
    float vertices[] = {
        // positions      // normals      // texcoords
        -0.1f, 0.0f, 0.0f, 0, 1, 0, 0, 0,
        0.1f, 0.0f, 0.0f, 0, 1, 0, 1, 0,
        0.0f, 0.5f, 0.0f, 0, 1, 0, 0.5, 1};
    vertexCount = 3;

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(3 * sizeof(float)));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(6 * sizeof(float)));

    glBindVertexArray(0);
}

void GrassField::setupInstancing()
{
    for (int x = 0; x < terrain.getWidth(); x += spacing)
    {
        for (int z = 0; z < terrain.getDepth(); z += spacing)
        {
            float height = terrain.getHeight(x, z);
            glm::vec3 color = terrain.getColor(x, z);
            if (color.g > 0.5f) // só onde tem grama
            {
                glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(x, height, z));
                float scale = 0.8f + static_cast<float>(rand()) / RAND_MAX * 0.4f;
                model = glm::scale(model, glm::vec3(scale));
                instanceMatrices.push_back(model);
            }
        }
    }

    glGenBuffers(1, &instanceVBO);
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    glBufferData(GL_ARRAY_BUFFER, instanceMatrices.size() * sizeof(glm::mat4), &instanceMatrices[0], GL_STATIC_DRAW);

    glBindVertexArray(VAO);
    std::size_t vec4Size = sizeof(glm::vec4);
    for (unsigned int i = 0; i < 4; i++)
    {
        glEnableVertexAttribArray(3 + i);
        glVertexAttribPointer(3 + i, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void *)(i * vec4Size));
        glVertexAttribDivisor(3 + i, 1);
    }
    glBindVertexArray(0);
}

void GrassField::Draw(const glm::mat4 &view, const glm::mat4 &projection)
{
    shader.use();
    shader.setMat4("view", view);
    shader.setMat4("projection", projection);

    glBindVertexArray(VAO);
    glDrawArraysInstanced(GL_TRIANGLES, 0, vertexCount, instanceMatrices.size());
    glBindVertexArray(0);
}
