#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include "Shader.hpp"
#include "Terrain.hpp"

class GrassField
{
public:
    GrassField(Terrain &terrain, Shader &shader, float spacing = 2.0f);
    ~GrassField();

    void Draw(const glm::mat4 &view, const glm::mat4 &projection);

private:
    void setupMesh();
    void setupInstancing();

    Shader &shader;
    Terrain &terrain;
    float spacing;

    unsigned int VAO, VBO, instanceVBO;
    std::vector<glm::mat4> instanceMatrices;
    unsigned int vertexCount;
};
