#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include "Shader.hpp"
#include "Terrain.hpp"
#include "Model.hpp"
#include "db_perlin.hpp" // <-- ADICIONE ESTA LINHA

class GrassField
{
public:
    GrassField(Terrain &terrain, Shader &shader, const std::string &modelPath, const std::string &texturePath, float spacing=3.0f);
    ~GrassField();

    void Draw(const glm::mat4 &view, const glm::mat4 &projection);

private:
    void setupInstancing();

    Terrain &terrain;
    Shader &shader;
    Model grassModel;
    float spacing;
    std::vector<glm::mat4> instanceMatrices;
    unsigned int instanceVBO;
};