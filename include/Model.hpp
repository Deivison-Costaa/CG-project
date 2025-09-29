#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <string>
#include <vector>
#include "Shader.hpp"
#include <glm/gtx/hash.hpp>

/**
 * @struct Vertex
 * @brief Estrutura que encapsula todos os atributos de um único vértice.
 * Combina posição, normal e coordenadas de textura em uma única unidade de dados,
 * facilitando o gerenciamento e o envio para a GPU.
 */
struct Vertex
{
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;

    // Sobrecarga do operador de igualdade, essencial para usar esta struct
    // como chave em um std::unordered_map para otimização de vértices.
    bool operator==(const Vertex &other) const
    {
        return Position == other.Position && Normal == other.Normal && TexCoords == other.TexCoords;
    }
};

// Especialização de std::hash para a struct Vertex
// Permite que a struct Vertex seja usada em estruturas de dados baseadas em hash,
// como std::unordered_map. Isso é crucial para a otimização que remove vértices duplicados.
namespace std
{
    template <>
    struct hash<Vertex>
    {
        size_t operator()(Vertex const &vertex) const
        {
            return ((hash<glm::vec3>()(vertex.Position) ^
                     (hash<glm::vec3>()(vertex.Normal) << 1)) >>
                    1) ^
                   (hash<glm::vec2>()(vertex.TexCoords) << 1);
        }
    };
}

/**
 * @class Model
 * @brief Representa um modelo 3D renderizável.
 * Esta classe gerencia o carregamento de dados de um arquivo .obj, o envio desses dados
 * para a GPU através de buffers (VBO, EBO, VAO) e a renderização do modelo.
 */
class Model
{
public:
    // O construtor carrega um modelo 3D e sua textura associada.
    Model(const std::string &path, const std::string &texturePath);
    ~Model(); // O destrutor é responsável por liberar os recursos da GPU.

    // Desenha o modelo na cena.
    void Draw(Shader &shader);

    // Getters para permitir que outras classes interajam com os dados do modelo (ex: para instancing).
    unsigned int getVAO();
    unsigned int getIndicesCount();

    // Ativa a textura do modelo para renderização.
    void bindTexture();

private:
    // Métodos privados que organizam a lógica interna da classe.

    // Carrega a geometria do modelo a partir de um arquivo .obj.
    void loadModel(const std::string &path);
    // Configura os buffers da GPU (VAO, VBO, EBO) com os dados do modelo.
    void setupMesh();
    // Carrega a textura a partir de um arquivo de imagem.
    void loadTexture(const std::string &path);

    // Dados do modelo armazenados na CPU.
    std::vector<Vertex> vertices;      // Lista de vértices únicos.
    std::vector<unsigned int> indices; // Ordem de desenho dos vértices para formar triângulos.

    // IDs dos objetos OpenGL na GPU.
    unsigned int VAO, VBO, EBO;
    unsigned int m_textureID; // ID da textura na GPU.
};