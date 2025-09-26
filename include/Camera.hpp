#ifndef CAMERA_H
#define CAMERA_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>

// Enum para possíveis movimentos da câmera
enum Camera_Movement
{
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT
};

// Valores padrão da câmera
const float YAW = -90.0f;
const float PITCH = 0.0f;
const float SPEED = 25.0f;
const float SENSITIVITY = 0.1f;
const float ZOOM = 45.0f;

class Camera
{
public:
    // Atributos da câmera
    glm::vec3 Position;
    glm::vec3 Front;
    glm::vec3 Up;
    glm::vec3 Right;
    glm::vec3 WorldUp;
    // Ângulos de Euler
    float Yaw;
    float Pitch;
    // Opções da câmera
    float MovementSpeed;
    float MouseSensitivity;
    float Zoom;

    // Construtor com vetores
    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH);

    // Retorna a matriz de visão calculada
    glm::mat4 GetViewMatrix();

    // Processa input do teclado
    void ProcessKeyboard(Camera_Movement direction, float deltaTime);

    // Processa input do mouse
    void ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true);

    // Processa input do scroll do mouse
    void ProcessMouseScroll(float yoffset);

private:
    // Recalcula os vetores da câmera a partir dos ângulos de Euler
    void updateCameraVectors();
};
#endif