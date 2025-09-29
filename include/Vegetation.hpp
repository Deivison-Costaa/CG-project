#ifndef VEGETATION_H
#define VEGETATION_H

#include <vector>
#include <glm/glm.hpp>
#include "Shader.hpp"
#include "Model.hpp"
#include "Terrain.hpp"

/**
 * @class Vegetation
 * @brief Gerencia a criação e renderização de um grande número de instâncias de um modelo.
 *
 * Esta classe utiliza a técnica de renderização instanciada para desenhar de forma eficiente
 * múltiplos objetos (como flores, rochas, etc.) na paisagem. Ela posiciona os objetos
 * aleatoriamente no terreno e os alinha com a normal da superfície para maior realismo.
 */
class Vegetation
{
public:
    /**
     * @brief Construtor da classe Vegetation.
     * @param terrain A referência ao terreno onde a vegetação será colocada.
     * @param shader A referência ao shader usado para renderizar a vegetação.
     * @param model A referência ao modelo 3D que será instanciado.
     * @param count O número desejado de instâncias a serem geradas.
     * @param minHeight A altura mínima no terreno para posicionar uma instância.
     * @param maxHeight A altura máxima no terreno para posicionar uma instância.
     * @param scale A escala a ser aplicada a cada instância.
     * @param modelUp O vetor que representa a direção "para cima" no modelo original (padrão é 0,1,0).
     */
    Vegetation(Terrain &terrain, Shader &shader, Model &model, int count, float minHeight, float maxHeight, float scale, glm::vec3 modelUp = glm::vec3(0.0f, 1.0f, 0.0f));
    ~Vegetation(); // Destrutor para liberar os recursos da GPU.

    /**
     * @brief Desenha todas as instâncias da vegetação.
     * @param view A matriz de visão da câmera.
     * @param projection A matriz de projeção da câmera.
     */
    void Draw(const glm::mat4 &view, const glm::mat4 &projection);

private:
    // Referências a objetos externos.
    Shader &m_shader;
    Model &m_model;

    // Propriedades das instâncias.
    int m_count; // O número final de instâncias geradas.
    unsigned int m_instanceVBO; // ID do VBO que armazena as matrizes de modelo.
    std::vector<glm::mat4> m_modelMatrices; // Lista de matrizes de transformação para cada instância.

    /**
     * @brief Configura o VBO de instâncias e os atributos de vértice no VAO do modelo.
     */
    void setupBuffers();
};

#endif