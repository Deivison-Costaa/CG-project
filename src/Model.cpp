#include "Model.hpp"
#include <iostream>
#include <unordered_map>

// Inclui e implementa as bibliotecas de cabeçalho único diretamente aqui.
#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

/**
 * @brief Construtor da classe Model.
 * Coordena o processo de carregamento do modelo e da textura, e a configuração dos buffers da GPU.
 */
Model::Model(const std::string &path, const std::string &texturePath)
{
    loadModel(path);
    loadTexture(texturePath);
    setupMesh();
}

/**
 * @brief Destrutor da classe Model.
 * Garante que todos os recursos alocados na GPU (buffers e texturas) sejam liberados
 * quando o objeto Model é destruído, prevenindo vazamentos de memória de vídeo.
 */
Model::~Model()
{
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteTextures(1, &m_textureID);
}

/**
 * @brief Carrega os dados de um arquivo .obj.
 * Utiliza a biblioteca tiny_obj_loader para analisar o arquivo. O principal desafio aqui é
 * combinar os diferentes atributos (posição, normal, texcoord) em uma única struct Vertex
 * e otimizar o resultado para remover vértices duplicados.
 */
void Model::loadModel(const std::string &path)
{
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    // Chama a função da biblioteca para carregar o arquivo.
    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.c_str()))
    {
        throw std::runtime_error(warn + err);
    }

    vertices.clear();
    indices.clear();

    // 'uniqueVertices' é um mapa usado para rastrear vértices que já foram processados.
    // Isso evita a duplicação de dados de vértices, otimizando o uso de memória da GPU.
    std::unordered_map<Vertex, uint32_t> uniqueVertices{};

    // Itera sobre todas as "formas" (objetos) dentro do arquivo .obj.
    for (const auto &shape : shapes)
    {
        // Itera sobre todos os índices que compõem as faces (triângulos) da forma.
        for (const auto &index : shape.mesh.indices)
        {
            Vertex vertex{}; // Cria uma nova struct Vertex para preencher.

            // Preenche a posição do vértice.
            vertex.Position = {
                attrib.vertices[3 * index.vertex_index + 0],
                attrib.vertices[3 * index.vertex_index + 1],
                attrib.vertices[3 * index.vertex_index + 2]};

            // Verifica se o vértice possui coordenadas de textura.
            if (index.texcoord_index >= 0)
            {
                vertex.TexCoords = {
                    attrib.texcoords[2 * index.texcoord_index + 0],
                    1.0f - attrib.texcoords[2 * index.texcoord_index + 1] // O Y é invertido, pois o formato .obj e o OpenGL têm origens diferentes.
                };
            }

            // Verifica se o vértice possui uma normal.
            if (index.normal_index >= 0)
            {
                vertex.Normal = {
                    attrib.normals[3 * index.normal_index + 0],
                    attrib.normals[3 * index.normal_index + 1],
                    attrib.normals[3 * index.normal_index + 2]};
            }

            // Lógica de otimização: se este vértice combinado ainda não foi visto...
            if (uniqueVertices.count(vertex) == 0)
            {
                // ... o adicionamos ao mapa com o índice atual...
                uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
                // ... e o adicionamos à nossa lista de vértices.
                vertices.push_back(vertex);
            }
            // Adicionamos o índice (seja novo ou reutilizado) à lista de índices para desenho.
            indices.push_back(uniqueVertices[vertex]);
        }
    }
}

/**
 * @brief Configura os buffers OpenGL (VAO, VBO, EBO).
 * Esta função envia os dados de vértices e índices da CPU para a memória da GPU
 * e especifica como a GPU deve interpretar esses dados durante a renderização.
 */
void Model::setupMesh()
{
    // 1. Gera e vincula o Vertex Array Object (VAO), que armazenará toda a configuração do estado deste modelo.
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    // 2. Envia os dados dos vértices para o Vertex Buffer Object (VBO).
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

    // 3. Envia os dados dos índices para o Element Buffer Object (EBO).
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

    // 4. Define como o pipeline gráfico deve interpretar os dados do VBO.
    // Atributo de Posição (layout = 0)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, Position));
    // Atributo de Normal (layout = 1)
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, Normal));
    // Atributo de Coordenadas de Textura (layout = 2)
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, TexCoords));

    // Desvincula o VAO para evitar modificações acidentais.
    glBindVertexArray(0);
}

/**
 * @brief Carrega uma textura de um arquivo de imagem.
 * Utiliza a biblioteca stb_image para carregar os dados da imagem e, em seguida,
 * configura uma textura 2D no OpenGL com esses dados.
 */
void Model::loadTexture(const std::string &path)
{
    glGenTextures(1, &m_textureID);
    glBindTexture(GL_TEXTURE_2D, m_textureID);

    int width, height, nrChannels;
    // Carrega a imagem do disco.
    unsigned char *data = stbi_load(path.c_str(), &width, &height, &nrChannels, 0);
    if (data)
    {
        // Determina o formato da imagem (RGB ou RGBA).
        GLenum format = GL_RGB;
        if (nrChannels == 4)
            format = GL_RGBA;

        // Envia os dados da imagem para a textura OpenGL vinculada.
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D); // Gera mipmaps para melhor qualidade de textura à distância.

        // Define os parâmetros de wrapping e filtering da textura.
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }
    else
    {
        std::cout << "Falha ao carregar a textura: " << path << std::endl;
    }
    // Libera a memória da imagem na CPU, pois os dados já estão na GPU.
    stbi_image_free(data);
}

/**
 * @brief Renderiza o modelo.
 * Esta função é chamada no loop principal de renderização. Ela vincula a textura e o VAO
 * do modelo e emite uma chamada de desenho para a GPU.
 */
void Model::Draw(Shader &shader)
{
    bindTexture(); // Ativa a textura do modelo.
    glBindVertexArray(VAO); // Ativa o VAO, restaurando todo o estado de renderização do modelo.

    // Comando para a GPU desenhar os triângulos usando os índices do EBO.
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, 0);

    // Desvincula o VAO como boa prática.
    glBindVertexArray(0);
}

// Implementação dos métodos 'getter'.
unsigned int Model::getVAO() { return VAO; }
unsigned int Model::getIndicesCount() { return indices.size(); }
void Model::bindTexture() { glBindTexture(GL_TEXTURE_2D, m_textureID); }