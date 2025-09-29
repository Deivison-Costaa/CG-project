#include "Shader.hpp"
#include <fstream>
#include <sstream>
#include <iostream>

/**
 * @brief Construtor da classe Shader.
 * Realiza todo o processo de carregar, compilar e ligar os shaders.
 */
Shader::Shader(const char *vertexPath, const char *fragmentPath)
{
    // 1. Obter o código-fonte do shader a partir dos caminhos dos ficheiros.
    std::string vertexCode;
    std::string fragmentCode;
    std::ifstream vShaderFile;
    std::ifstream fShaderFile;

    // Garante que os objetos ifstream podem lançar exceções em caso de falha.
    vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    try
    {
        // Abre os ficheiros.
        vShaderFile.open(vertexPath);
        fShaderFile.open(fragmentPath);
        std::stringstream vShaderStream, fShaderStream;
        // Lê o conteúdo dos ficheiros para os streams.
        vShaderStream << vShaderFile.rdbuf();
        fShaderStream << fShaderFile.rdbuf();
        // Fecha os ficheiros.
        vShaderFile.close();
        fShaderFile.close();
        // Converte os streams em strings.
        vertexCode = vShaderStream.str();
        fragmentCode = fShaderStream.str();
    }
    catch (std::ifstream::failure &e)
    {
        std::cerr << "ERRO::SHADER::ARQUIVO_NAO_LIDO_COM_SUCESSO" << std::endl;
    }
    const char *vShaderCode = vertexCode.c_str();
    const char *fShaderCode = fragmentCode.c_str();

    // 2. Compilar os shaders.
    unsigned int vertex, fragment;

    // Vertex Shader
    vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vShaderCode, NULL);
    glCompileShader(vertex);
    checkCompileErrors(vertex, "VERTEX"); // Verifica se a compilação foi bem-sucedida.

    // Fragment Shader
    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fShaderCode, NULL);
    glCompileShader(fragment);
    checkCompileErrors(fragment, "FRAGMENT"); // Verifica se a compilação foi bem-sucedida.

    // 3. Criar e ligar o Programa de Shader.
    ID = glCreateProgram();
    glAttachShader(ID, vertex);
    glAttachShader(ID, fragment);
    glLinkProgram(ID);
    checkCompileErrors(ID, "PROGRAM"); // Verifica se a ligação foi bem-sucedida.

    // 4. Eliminar os objetos de shader individuais, pois já não são necessários após a ligação.
    glDeleteShader(vertex);
    glDeleteShader(fragment);
}

/**
 * @brief Ativa o programa de shader.
 */
void Shader::use()
{
    glUseProgram(ID);
}

// Implementação das funções de definição de uniforms
//  Cada função obtém a localização da variável uniform no programa de shader através do seu nome
//  e, em seguida, define o seu valor usando a chamada apropriada do OpenGL (glUniform...).

void Shader::setBool(const std::string &name, bool value) const
{
    glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
}

void Shader::setInt(const std::string &name, int value) const
{
    glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
}

void Shader::setFloat(const std::string &name, float value) const
{
    glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
}

void Shader::setVec2(const std::string &name, const glm::vec2 &value) const
{
    glUniform2fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
}

void Shader::setVec3(const std::string &name, const glm::vec3 &value) const
{
    glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
}

void Shader::setVec4(const std::string &name, const glm::vec4 &value) const
{
    glUniform4fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
}

void Shader::setMat4(const std::string &name, const glm::mat4 &mat) const
{
    glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}

/**
 * @brief Função utilitária para verificar erros de compilação/ligação.
 */
void Shader::checkCompileErrors(unsigned int shader, std::string type)
{
    int success;
    char infoLog[1024];
    if (type != "PROGRAM") // Verifica erros de compilação de um shader individual.
    {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(shader, 1024, NULL, infoLog);
            std::cerr << "ERRO::SHADER_COMPILATION_ERROR de tipo: " << type << "\n"
                      << infoLog << "\n----------------------------------------------- -- " << std::endl;
        }
    }
    else // Verifica erros de ligação do programa de shader.
    {
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if (!success)
        {
            glGetProgramInfoLog(shader, 1024, NULL, infoLog);
            std::cerr << "ERRO::PROGRAM_LINKING_ERROR de tipo: " << type << "\n"
                      << infoLog << "\n----------------------------------------------- -- " << std::endl;
        }
    }
}