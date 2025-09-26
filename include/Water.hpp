#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include "Shader.hpp"

class Water
{
public:
    Water(float width, float depth, Shader &shader);
    ~Water();

    void Draw(const glm::mat4 &view, const glm::mat4 &projection, float time);

private:
    void setupMesh();

    float width, depth;
    Shader &shader;

    unsigned int VAO, VBO, EBO;
};
