#ifndef TERRAIN_H
#define TERRAIN_H

#include "Shader.hpp"
#include <vector>
#include <string>
#include <glm/glm.hpp>

/**
 * @class Terrain
 * @brief Gerencia a geração procedural, texturização e renderização do terreno.
 *
 * Esta classe cria uma malha de terreno baseada em Ruído de Perlin,
 * aplica múltiplas texturas (areia, grama, rocha) que são misturadas no shader
 * com base na altura, e fornece informações sobre sua geometria para outros objetos.
 */
class Terrain
{
public:
    /**
     * @brief Construtor da classe Terrain.
     * @param width A largura do terreno na grade de vértices.
     * @param depth A profundidade do terreno na grade de vértices.
     * @param shader A referência ao shader que será usado para renderizar o terreno.
     * @param sandTexturePath O caminho para a textura de areia.
     * @param grassTexturePath O caminho para a textura de grama.
     * @param rockTexturePath O caminho para a textura de rocha.
     */
    Terrain(int width, int depth, Shader &shader, const std::string &sandTexturePath, const std::string &grassTexturePath, const std::string &rockTexturePath);
    ~Terrain(); // Destrutor para liberar os recursos da GPU.

    /**
     * @brief Desenha o terreno na cena.
     * @param view A matriz de visão da câmera.
     * @param projection A matriz de projeção da câmera.
     */
    void Draw(const glm::mat4 &view, const glm::mat4 &projection);

    // Getters
    //  Funções essenciais para que outros objetos possam interagir com o terreno.
    int getWidth() const;
    int getDepth() const;
    float getHeight(int x, int z) const;
    glm::vec3 getNormal(float x, float z) const;

private:
    // Dimensões da grade do terreno.
    int m_width, m_depth;
    // IDs dos objetos OpenGL.
    unsigned int m_VAO, m_VBO, m_EBO;
    unsigned int m_indexCount;
    // Referência ao shader do terreno.
    Shader &m_shader;

    // Cache das alturas do terreno para acesso rápido.
    std::vector<float> m_heights;

    // IDs das texturas na GPU.
    unsigned int m_grassTextureID;
    unsigned int m_rockTextureID;
    unsigned int m_sandTextureID;

    /**
     * @brief Configura os buffers da GPU (VAO, VBO, EBO) com a geometria do terreno.
     */
    void setupTerrain(const std::vector<float> &vertices, const std::vector<unsigned int> &indices);

    /**
     * @brief Carrega uma textura a partir de um arquivo e retorna seu ID OpenGL.
     */
    unsigned int loadTexture(const char *path);

    // Funções Auxiliares para Geração Procedural

    /**
     * @brief Calcula a altura (coordenada Y) de um ponto no terreno usando Ruído de Perlin.
     */
    float calculateHeight(float x, float z) const;

    /**
     * @brief Calcula a normal da superfície em um ponto do terreno.
     */
    glm::vec3 calculateNormal(int x, int z) const;
};

#endif