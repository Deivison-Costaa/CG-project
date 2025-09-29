#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <vector>
#include <algorithm>

#include "Shader.hpp"
#include "Camera.hpp"
#include "Terrain.hpp"
#include "Sun.hpp"
#include "Water.hpp"
#include "GrassField.hpp"
#include "Vegetation.hpp"
#include "WaterFrameBuffers.hpp" // Inclui a nova classe
#include <stb_image.h>

// Protótipos das callbacks e funções auxiliares
void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void mouse_callback(GLFWwindow *window, double xpos, double ypos);
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);
unsigned int loadTexture(const char *path);
void renderScene(const glm::vec4 &clipPlane, const glm::mat4 &view, const glm::mat4 &projection,
                 Terrain &terrain, Sun &sun, GrassField &grass,
                 std::vector<std::reference_wrapper<Vegetation>> &vegetation, // <-- MUDANÇA AQUI
                 Shader &terrainShader, Shader &sunShader, Shader &grassShader, Shader &vegetationShader);

glm::vec3 getPathPosition(float t, bool &finished);

// Configurações da Janela e do Mundo
const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;
const float WATER_HEIGHT = -13.0f; // Coordenada Y da superfície da água

// Estado da Câmara
Camera camera(glm::vec3(0.0f, 30.0f, 100.0f)); // Objeto da câmera principal
float lastX = SCR_WIDTH / 2.0f;                // Guarda a última posição X do rato
float lastY = SCR_HEIGHT / 2.0f;               // Guarda a última posição Y do rato
bool firstMouse = true;                        // Controla o primeiro movimento do rato

// Controlo de Tempo
float deltaTime = 0.0f; // Tempo decorrido entre o frame atual e o anterior
float lastFrame = 0.0f; // Registo de tempo do último frame

// Ciclo Dia/Noite
float sunTime = 0.25f;    // Progresso do ciclo diário (0.0 a 1.0)
float timeScale = 0.01f; // Multiplicador da velocidade do tempo

// Animação da Água
float waterMoveFactor = 0.0f;  // Deslocamento para a textura de distorção da água
const float WAVE_SPEED = 1.3f; // Velocidade de movimento das ondas

// Modo Cinemático
bool cinematicMode = false;           // Flag para ativar/desativar a câmara cinemática
float pathTime = 0.0f;                // Posição atual (parâmetro 't') no caminho da câmara
float pathSpeed = 10.0f;              // Velocidade da câmara cinemática
float y_offset = 30.0f;               // Elevação adicional para os pontos de controlo
std::vector<glm::vec3> controlPoints; // Pontos que definem o caminho da câmara

int main()
{
    // Inicialização
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "projeto", NULL, NULL);
    if (window == NULL)
    {
        std::cerr << "Falha ao criar janela GLFW" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "Falha ao inicializar GLAD" << std::endl;
        return -1;
    }

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CLIP_DISTANCE0); // Habilita o clipping plane para a água
    // esse escopo aqui tá aberto por que o glfwterminate tava finalizando tudo antes dos destrutores serem chamados,
    // causando falha de segmentação, isso aqui garante que isso não ocorra.
    {
        // Shaders e Objetos
        Shader terrainShader("shaders/terrain.vert", "shaders/terrain.frag");
        Shader sunShader("shaders/sun.vert", "shaders/sun.frag");
        Shader waterShader("shaders/water.vert", "shaders/water.frag");
        Shader grassShader("shaders/grass.vert", "shaders/grass.frag");
        Shader vegetationShader("shaders/vegetation.vert", "shaders/vegetation.frag");

        // Modelos e Texturas
        Model flowerModel("models/anemona.obj", "textures/anemona.jpg");
        Model flowerModel1("models/flor1.obj", "textures/flor1.jpg");
        unsigned int dudvTexture = loadTexture("textures/waterDUDV.png");
        unsigned int normalMapTexture = loadTexture("textures/waterNormalMap.png");

        // Instâncias dos objetos
        Terrain terrain(512, 512, terrainShader, "textures/mar.png", "textures/grass8.png", "textures/rock1.png");
        Sun sun(sunShader);
        Water water(terrain.getWidth(), terrain.getDepth(), waterShader);
        GrassField grass(terrain, grassShader, "models/Grass1.obj", "textures/Grass/Grass08.png");
        Vegetation flowers(terrain, vegetationShader, flowerModel, 500, -5.0f, 4.0f, 0.3f, glm::vec3(0.0f, 0.0f, 1.0f));
        Vegetation flowers1(terrain, vegetationShader, flowerModel1, 500, -5.0f, 4.0f, 0.7f, glm::vec3(0.0f, 0.0f, 1.0f));

        std::vector<std::reference_wrapper<Vegetation>> allVegetation;
        allVegetation.push_back(flowers);
        allVegetation.push_back(flowers1);

        WaterFrameBuffers fbos;

        // Define os pontos de controle para a câmera cinemática
        glm::vec3 startPoint = glm::vec3(0, terrain.getHeight(256, 256) + y_offset, 150);
        controlPoints.push_back(startPoint);
        controlPoints.push_back(startPoint);
        controlPoints.push_back(glm::vec3(100, terrain.getHeight(256 + 100, 256 + 50) + y_offset, 50));
        controlPoints.push_back(glm::vec3(200, terrain.getHeight(256 + 200, 256 - 100) + 40.0f, -100));
        controlPoints.push_back(glm::vec3(50, terrain.getHeight(256 + 50, 256 - 200) + y_offset, -200));
        controlPoints.push_back(glm::vec3(-150, terrain.getHeight(256 - 150, 256 - 150) + y_offset, -150));
        controlPoints.push_back(glm::vec3(-200, terrain.getHeight(256 - 200, 256 + 50) + 35.0f, 50));
        controlPoints.push_back(glm::vec3(-100, terrain.getHeight(256 - 100, 256 + 180) + y_offset, 180));
        glm::vec3 endPoint = glm::vec3(0, terrain.getHeight(256, 256 + 150) + y_offset, 150);
        controlPoints.push_back(endPoint);
        controlPoints.push_back(endPoint);

        // Loop de Renderização
        while (!glfwWindowShouldClose(window))
        {
            // Timing
            float currentFrame = static_cast<float>(glfwGetTime());
            deltaTime = currentFrame - lastFrame;
            lastFrame = currentFrame;

            // Input
            processInput(window);

            // Atualizações
            sunTime += timeScale * deltaTime;
            sunTime = fmodf(sunTime, 1.0f);
            sun.Update(sunTime);
            waterMoveFactor += WAVE_SPEED * deltaTime;
            waterMoveFactor = fmod(waterMoveFactor, 1.0f);

            // Atualiza a câmera no modo cinemático
            if (cinematicMode)
            {
                pathTime += (pathSpeed / 100.0f) * deltaTime;
                bool finished = false;
                glm::vec3 currentPos = getPathPosition(pathTime, finished);
                glm::vec3 nextPos = getPathPosition(pathTime + 0.01f, finished);
                if (finished)
                {
                    cinematicMode = false;
                }
                else
                {
                    camera.SetLookAt(currentPos, nextPos);
                }
            }

            // Matrizes de Projeção e Visão
            glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 2000.0f);
            glm::mat4 view = camera.GetViewMatrix();

            // RENDERIZAÇÃO EM MÚLTIPLAS PASSAGENS

            // 1. PASSAGEM DE REFLEXÃO (desenhar para o FBO de reflexão)
            fbos.bindReflectionFrameBuffer();
            float distance = 2 * (camera.Position.y - WATER_HEIGHT);
            camera.Position.y -= distance;
            camera.InvertPitch();
            glm::mat4 reflectionView = camera.GetViewMatrix();

            renderScene(glm::vec4(0, 1, 0, -WATER_HEIGHT + 0.1f), reflectionView, projection, terrain, sun, grass, allVegetation, terrainShader, sunShader, grassShader, vegetationShader);

            camera.Position.y += distance;
            camera.InvertPitch();

            // 2. PASSAGEM DE REFRAÇÃO (desenhar para o FBO de refração)
            fbos.bindRefractionFrameBuffer();
            renderScene(glm::vec4(0, -1, 0, WATER_HEIGHT), view, projection, terrain, sun, grass, allVegetation, terrainShader, sunShader, grassShader, vegetationShader);

            // 3. PASSAGEM PRINCIPAL (desenhar para o ecrã)
            fbos.unbindCurrentFrameBuffer(SCR_WIDTH, SCR_HEIGHT);
            renderScene(glm::vec4(0, 0, 0, 0), view, projection, terrain, sun, grass, allVegetation, terrainShader, sunShader, grassShader, vegetationShader);

            // FINALMENTE, DESENHAR A ÁGUA
            waterShader.use();
            waterShader.setMat4("projection", projection);
            waterShader.setMat4("view", view);
            waterShader.setVec3("cameraPos", camera.Position);
            waterShader.setVec3("lightDir", sun.GetLightDirection());
            waterShader.setVec3("lightColor", sun.GetLightColor());
            waterShader.setFloat("moveFactor", waterMoveFactor);

            glm::mat4 waterModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0, WATER_HEIGHT, 0));
            waterShader.setMat4("model", waterModelMatrix);

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, fbos.getReflectionTexture());
            waterShader.setInt("reflectionTexture", 0);

            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, fbos.getRefractionTexture());
            waterShader.setInt("refractionTexture", 1);

            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_2D, dudvTexture);
            waterShader.setInt("dudvMap", 2);

            glActiveTexture(GL_TEXTURE3);
            glBindTexture(GL_TEXTURE_2D, normalMapTexture);
            waterShader.setInt("normalMap", 3);

            water.Draw(glm::translate(glm::mat4(1.0f), glm::vec3(0, WATER_HEIGHT, 0)));

            glfwSwapBuffers(window);
            glfwPollEvents();
        }
    }

    glfwTerminate();
    return 0;
}

// Função auxiliar para desenhar a cena inteira
void renderScene(const glm::vec4 &clipPlane, const glm::mat4 &view, const glm::mat4 &projection,
                 Terrain &terrain, Sun &sun, GrassField &grass, std::vector<std::reference_wrapper<Vegetation>> &vegetation,
                 Shader &terrainShader, Shader &sunShader, Shader &grassShader, Shader &vegetationShader)
{
    glm::vec3 skyColor = sun.GetSkyColor();
    glm::vec3 lightDir = sun.GetLightDirection();
    glm::vec3 lightColor = sun.GetLightColor();

    glClearColor(skyColor.r, skyColor.g, skyColor.b, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // 1. Terreno
    terrainShader.use();
    terrainShader.setMat4("view", view);
    terrainShader.setMat4("projection", projection);
    terrainShader.setVec3("viewPos", camera.Position);
    terrainShader.setVec3("lightDir", lightDir);
    terrainShader.setVec3("lightColor", lightColor);
    terrainShader.setFloat("terrainAmplitude", 50.0f);
    terrainShader.setVec4("plane", clipPlane);
    terrain.Draw(view, projection);

    // 2. Sol
    sunShader.use();
    sunShader.setVec4("plane", clipPlane);
    sun.Draw(view, projection);

    // 3. Grama
    grassShader.use();
    grassShader.setMat4("view", view);
    grassShader.setMat4("projection", projection);
    grassShader.setVec3("viewPos", camera.Position);
    grassShader.setVec3("lightDir", lightDir);
    grassShader.setVec3("lightColor", lightColor);
    grassShader.setVec4("plane", clipPlane);
    grass.Draw(view, projection);

    // 4. Vegetação
    vegetationShader.use();
    vegetationShader.setMat4("view", view);
    vegetationShader.setMat4("projection", projection);
    vegetationShader.setVec3("viewPos", camera.Position);
    vegetationShader.setVec3("lightDir", lightDir);
    vegetationShader.setVec3("lightColor", lightColor);
    vegetationShader.setVec4("plane", clipPlane);
    for (Vegetation &veg : vegetation)
    {
        veg.Draw(view, projection);
    }
}

void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // Ativa/Desativa o modo cinemático com a tecla C
    static bool c_key_pressed = false;
    if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS && !c_key_pressed)
    {
        cinematicMode = !cinematicMode;
        pathTime = 0.0f; // Reinicia o caminho
    }
    c_key_pressed = (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS);

    // Só processa o input do teclado se não estiver no modo cinemático
    if (!cinematicMode)
    {
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            camera.ProcessKeyboard(FORWARD, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            camera.ProcessKeyboard(BACKWARD, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            camera.ProcessKeyboard(LEFT, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            camera.ProcessKeyboard(RIGHT, deltaTime);
    }
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow *window, double xposIn, double yposIn)
{
    if (cinematicMode)
    {
        firstMouse = true;
        return;
    }

    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

unsigned int loadTexture(const char *path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format = GL_RGB;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}

glm::vec3 catmullRom(float t, const glm::vec3 &p0, const glm::vec3 &p1, const glm::vec3 &p2, const glm::vec3 &p3)
{
    float t2 = t * t;
    float t3 = t2 * t;
    return 0.5f * ((2.0f * p1) +
                   (-p0 + p2) * t +
                   (2.0f * p0 - 5.0f * p1 + 4.0f * p2 - p3) * t2 +
                   (-p0 + 3.0f * p1 - 3.0f * p2 + p3) * t3);
}

glm::vec3 getPathPosition(float t, bool &finished)
{
    if (controlPoints.size() < 4)
    {
        finished = true;
        return glm::vec3(0.0f);
    }

    int numSegments = controlPoints.size() - 3;
    float totalPathTime = (float)numSegments;

    if (t >= totalPathTime)
    {
        finished = true;
        return controlPoints[controlPoints.size() - 2];
    }

    finished = false;
    int currentSegment = static_cast<int>(floor(t));
    float local_t = t - currentSegment;

    const glm::vec3 &p0 = controlPoints[currentSegment];
    const glm::vec3 &p1 = controlPoints[currentSegment + 1];
    const glm::vec3 &p2 = controlPoints[currentSegment + 2];
    const glm::vec3 &p3 = controlPoints[currentSegment + 3];

    return catmullRom(local_t, p0, p1, p2, p3);
}