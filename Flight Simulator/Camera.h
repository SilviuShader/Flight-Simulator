#pragma once

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

class Camera
{
private:

    const float CAMERA_MOVE_SPEED = 10.0f;
    const float CAMERA_ROTATE_SPEED = 0.001f;
    const float MINIMUM_TRANSLATION_BIAS = 0.01f;

public:

    Camera(float, float, float, float, float);

    void      ProcessKeybaordInput(GLFWwindow*);
    void      ProcessMouseInput(float, float);
    void      UpdateWindowSize(float, float);

    void      Update(float);

    glm::mat4 GetViewMatrix()       const;
    glm::mat4 GetProjectionMatrix() const;
    glm::vec3 GetPosition()         const;

private:

    glm::mat4 GetRotationMatrix();
    void      UpdateViewMatrix();

private:

    float     m_fieldOfView;
    float     m_near;
    float     m_far;

    glm::vec3 m_position;
    glm::vec3 m_rotation;

    glm::mat4 m_viewMatrix;
    glm::mat4 m_projectionMatrix;

    bool      m_upPressed;
    bool      m_leftPressed;
    bool      m_downPressed;
    bool      m_rightPressed;
};