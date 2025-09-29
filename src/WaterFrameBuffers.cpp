#include "WaterFrameBuffers.hpp"
#include <iostream>

/**
 * @brief Construtor que inicializa ambos os FBOs.
 */
WaterFrameBuffers::WaterFrameBuffers()
{
    initialiseReflectionFrameBuffer();
    initialiseRefractionFrameBuffer();
}

/**
 * @brief Destrutor que liberta todos os recursos OpenGL alocados (FBOs, texturas, renderbuffers).
 * Isto é crucial para evitar fugas de memória de vídeo.
 */
WaterFrameBuffers::~WaterFrameBuffers()
{
    glDeleteFramebuffers(1, &m_reflectionFrameBuffer);
    glDeleteTextures(1, &m_reflectionTexture);
    glDeleteRenderbuffers(1, &m_reflectionDepthBuffer);
    glDeleteFramebuffers(1, &m_refractionFrameBuffer);
    glDeleteTextures(1, &m_refractionTexture);
    glDeleteTextures(1, &m_refractionDepthTexture);
}

/**
 * @brief Ativa o FBO de reflexão para ser o alvo de renderização.
 */
void WaterFrameBuffers::bindReflectionFrameBuffer()
{
    bindFrameBuffer(m_reflectionFrameBuffer, REFLECTION_WIDTH, REFLECTION_HEIGHT);
}

/**
 * @brief Ativa o FBO de refração para ser o alvo de renderização.
 */
void WaterFrameBuffers::bindRefractionFrameBuffer()
{
    bindFrameBuffer(m_refractionFrameBuffer, REFRACTION_WIDTH, REFRACTION_HEIGHT);
}

/**
 * @brief Desvincula o FBO atual, retornando a renderização para o framebuffer padrão (o ecrã).
 */
void WaterFrameBuffers::unbindCurrentFrameBuffer(int screenWidth, int screenHeight)
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);        // O ID 0 representa o framebuffer padrão.
    glViewport(0, 0, screenWidth, screenHeight); // Restaura o viewport para o tamanho do ecrã.
}

/**
 * @brief Cria e configura o FBO de reflexão e os seus anexos.
 */
void WaterFrameBuffers::initialiseReflectionFrameBuffer()
{
    m_reflectionFrameBuffer = createFrameBuffer();
    m_reflectionTexture = createTextureAttachment(REFLECTION_WIDTH, REFLECTION_HEIGHT);
    // Para a reflexão, um Renderbuffer de profundidade é suficiente, pois não precisamos de ler os valores de profundidade depois.
    m_reflectionDepthBuffer = createDepthBufferAttachment(REFLECTION_WIDTH, REFLECTION_HEIGHT);
    unbindCurrentFrameBuffer(1280, 720);
}

/**
 * @brief Cria e configura o FBO de refração e os seus anexos.
 */
void WaterFrameBuffers::initialiseRefractionFrameBuffer()
{
    m_refractionFrameBuffer = createFrameBuffer();
    m_refractionTexture = createTextureAttachment(REFRACTION_WIDTH, REFRACTION_HEIGHT);
    // Para a refração, usamos uma Textura de profundidade. Isto permite que o shader da água
    // aceda aos valores de profundidade para simular efeitos como o escurecimento da água com a profundidade.
    m_refractionDepthTexture = createDepthTextureAttachment(REFRACTION_WIDTH, REFRACTION_HEIGHT);
    unbindCurrentFrameBuffer(1280, 720);
}

/**
 * @brief Função auxiliar para vincular um FBO e ajustar o viewport.
 */
void WaterFrameBuffers::bindFrameBuffer(int frameBuffer, int width, int height)
{
    glBindTexture(GL_TEXTURE_2D, 0); // Garante que nenhuma textura está vinculada antes de mudar o alvo de renderização.
    glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
    glViewport(0, 0, width, height); // Ajusta o viewport para as dimensões do FBO.
}

/**
 * @brief Cria um objeto Framebuffer.
 */
unsigned int WaterFrameBuffers::createFrameBuffer()
{
    unsigned int frameBuffer;
    glGenFramebuffers(1, &frameBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
    // Indica que vamos desenhar para o anexo de cor 0.
    glDrawBuffer(GL_COLOR_ATTACHMENT0);
    return frameBuffer;
}

/**
 * @brief Cria uma textura para ser usada como anexo de cor de um FBO.
 */
unsigned int WaterFrameBuffers::createTextureAttachment(int width, int height)
{
    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    // Aloca memória para a textura na GPU, mas não a preenche com dados (nullptr).
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    // Anexa a textura ao FBO atualmente vinculado.
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, texture, 0);
    return texture;
}

/**
 * @brief Cria uma textura para ser usada como anexo de profundidade de um FBO.
 */
unsigned int WaterFrameBuffers::createDepthTextureAttachment(int width, int height)
{
    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    // Anexa como o anexo de profundidade do FBO.
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, texture, 0);
    return texture;
}

/**
 * @brief Cria um Renderbuffer para ser usado como anexo de profundidade de um FBO.
 * Renderbuffers são mais eficientes que texturas quando não se precisa de ler os dados de volta.
 */
unsigned int WaterFrameBuffers::createDepthBufferAttachment(int width, int height)
{
    unsigned int depthBuffer;
    glGenRenderbuffers(1, &depthBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);
    // Aloca o armazenamento para o renderbuffer.
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
    // Anexa o renderbuffer ao FBO.
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);
    return depthBuffer;
}