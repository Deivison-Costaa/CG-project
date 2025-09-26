#include "Water.hpp"
#include <vector>

Water::Water(float w, float d, Shader &sh) : width(w), depth(d), shader(sh)
{
    setupMesh();
}

Water::~Water()
{
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
}

void Water::setupMesh()
{
    float halfW = width / 2.0f;
    float halfD = depth / 2.0f;

    float vertices[] = {
        -halfW, 0.0f, -halfD, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
        halfW, 0.0f, -halfD, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
        halfW, 0.0f, halfD, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,
        -halfW, 0.0f, halfD, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f};

    unsigned int indices[] = {0, 1, 2, 2, 3, 0};

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // posição
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);
    // normal
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // texcoords
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
}

void Water::Draw(const glm::mat4 &view, const glm::mat4 &projection, float time)
{
    shader.use();
    
    glm::mat4 model = glm::mat4(1.0f); // identidade
    shader.setMat4("model", model);
    shader.setMat4("view", view);
    shader.setMat4("projection", projection);
    shader.setFloat("time", time); // para ondulações animadas

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}
