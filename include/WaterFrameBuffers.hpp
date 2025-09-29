#ifndef WATERFRAMEBUFFERS_H
#define WATERFRAMEBUFFERS_H

#include <glad/glad.h>

/**
 * @class WaterFrameBuffers
 * @brief Gerencia os Framebuffer Objects (FBOs) para os efeitos de render-to-texture da água.
 *
 * Esta classe encapsula a criação e gestão de dois FBOs: um para a
 * textura de reflexão e outro para a textura de refração. Estes FBOs permitem
 * renderizar a cena "off-screen" para texturas que são depois usadas pelo
 * shader da água para criar efeitos realistas.
 */
class WaterFrameBuffers
{
public:
    WaterFrameBuffers();
    ~WaterFrameBuffers(); // Essencial para libertar os recursos da GPU.

    // Ativa o FBO de reflexão como o alvo de renderização.
    void bindReflectionFrameBuffer();
    // Ativa o FBO de refração como o alvo de renderização.
    void bindRefractionFrameBuffer();
    // Desativa qualquer FBO e volta a renderizar para o ecrã principal.
    void unbindCurrentFrameBuffer(int screenWidth, int screenHeight);

    // Getters para obter os IDs das texturas geradas.
    int getReflectionTexture() const { return m_reflectionTexture; }
    int getRefractionTexture() const { return m_refractionTexture; }
    int getRefractionDepthTexture() const { return m_refractionDepthTexture; }

private:
    // Constantes de Configuração
    //  É uma otimização comum renderizar a reflexão numa resolução mais baixa,
    //  pois pequenas imperfeições são menos notáveis devido às ondas.
    static const int REFLECTION_WIDTH = 320;
    static const int REFLECTION_HEIGHT = 180;
    // A refração é renderizada na resolução total para manter a clareza do que está debaixo de água.
    static const int REFRACTION_WIDTH = 1280;
    static const int REFRACTION_HEIGHT = 720;

    // Recursos do FBO de Reflexão
    unsigned int m_reflectionFrameBuffer;
    unsigned int m_reflectionTexture;     // Anexo de cor (o resultado da renderização).
    unsigned int m_reflectionDepthBuffer; // Anexo de profundidade (para o teste de profundidade).

    // Recursos do FBO de Refração
    unsigned int m_refractionFrameBuffer;
    unsigned int m_refractionTexture;      // Anexo de cor.
    unsigned int m_refractionDepthTexture; // Anexo de profundidade (como uma textura).

    // Métodos de Inicialização
    void initialiseReflectionFrameBuffer();
    void initialiseRefractionFrameBuffer();

    // Métodos Auxiliares de Baixo Nível
    void bindFrameBuffer(int frameBuffer, int width, int height);
    unsigned int createFrameBuffer();
    unsigned int createTextureAttachment(int width, int height);
    unsigned int createDepthTextureAttachment(int width, int height);
    unsigned int createDepthBufferAttachment(int width, int height);
};

#endif