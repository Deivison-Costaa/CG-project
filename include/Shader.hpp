#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <string>

/**
 * @class Shader
 * @brief Gerencia a compilação, ligação e utilização de programas de shader do OpenGL.
 *
 * Esta classe abstrai o processo de carregar shaders de vertex e fragment a partir de ficheiros,
 * compilá-los, ligá-los a um programa de shader e fornecer uma interface
 * fácil para ativar o programa e definir as suas variáveis 'uniform'.
 */
class Shader
{
public:
    // O ID de referência do programa de shader no OpenGL.
    unsigned int ID;

    /**
     * @brief Construtor que lê e constrói o programa de shader.
     * @param vertexPath O caminho do ficheiro para o código-fonte do vertex shader.
     * @param fragmentPath O caminho do ficheiro para o código-fonte do fragment shader.
     */
    Shader(const char *vertexPath, const char *fragmentPath);

    /**
     * @brief Ativa este programa de shader para ser usado nas subsequentes chamadas de renderização.
     */
    void use();

    // Funções Utilitárias para Definir Uniforms
    //  Estas funções permitem enviar dados do código C++ para as variáveis uniform nos shaders.
    void setBool(const std::string &name, bool value) const;
    void setInt(const std::string &name, int value) const;
    void setFloat(const std::string &name, float value) const;
    void setVec2(const std::string &name, const glm::vec2 &value) const;
    void setVec3(const std::string &name, const glm::vec3 &value) const;
    void setVec4(const std::string &name, const glm::vec4 &value) const;
    void setMat4(const std::string &name, const glm::mat4 &mat) const;

private:
    /**
     * @brief Verifica erros de compilação ou de ligação de shaders.
     * @param shader O ID do objeto shader ou do programa a ser verificado.
     * @param type O tipo de objeto ("VERTEX", "FRAGMENT" ou "PROGRAM") para contextualizar a mensagem de erro.
     */
    void checkCompileErrors(unsigned int shader, std::string type);
};

#endif