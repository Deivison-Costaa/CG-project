#ifndef TERRAIN_H
#define TERRAIN_H

#include "Shader.hpp"
#include <vector>
#include <glm/glm.hpp>

class Terrain
{
public:
    Terrain(int width, int depth, Shader &shader);
    ~Terrain();

    void Draw(const glm::mat4 &view, const glm::mat4 &projection);

    int getWidth() const;
    int getDepth() const;

    // NOVOS MÉTODOS
    float getHeight(int x, int z) const;
    glm::vec3 getColor(int x, int z) const;

private:
    int m_width, m_depth;
    unsigned int m_VAO, m_VBO, m_EBO;
    unsigned int m_indexCount;
    Shader &m_shader;

    std::vector<float> m_heights; // armazena todas as alturas

    void setupTerrain(const std::vector<float> &vertices, const std::vector<unsigned int> &indices);

    // Helpers privados
    float calculateHeight(float x, float z) const;
    glm::vec3 calculateNormal(int x, int z) const;
};

#endif
