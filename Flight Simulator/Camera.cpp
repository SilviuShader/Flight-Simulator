#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Camera.h"

using namespace glm;

Camera::Camera() :
    m_position(vec3(0.0f, 0.0f, 0.0f)),
    m_rotation(vec3(0.0f, 0.0f, 0.0f)),
    m_upPressed(false),
    m_leftPressed(false),
    m_downPressed(false),
    m_rightPressed(false)
{
    UpdateViewMatrix();
}

void Camera::ProcessKeybaordInput(GLFWwindow* window)
{
    m_upPressed = glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS;
    m_leftPressed = glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS;
    m_downPressed = glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS;
    m_rightPressed = glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS;
}

void Camera::ProcessMouseInput(float diffX, float diffY)
{
    m_rotation.x -= diffY * CAMERA_ROTATE_SPEED;
    m_rotation.y -= diffX * CAMERA_ROTATE_SPEED;

    constexpr float halfPi = half_pi<float>();

    if (m_rotation.x >= halfPi)
        m_rotation.x = halfPi;
    if (m_rotation.x <= -halfPi)
        m_rotation.x = halfPi;

    UpdateViewMatrix();
}

void Camera::Update(float deltaTime)
{
    mat4 rotationMatrix = GetRotationMatrix();

    vec4 translation = vec4(0.0f, 0.0f, 0.0f, 0.0f);
    bool translated = false;

    if (m_upPressed)
    {
        translation = vec4(translation.x, translation.y, translation.z - 1.0f, translation.w);
        translated = true;
    }

    if (m_leftPressed)
    {
        translation = vec4(translation.x - 1.0f, translation.y, translation.z, translation.w);
        translated = true;
    }

    if (m_downPressed)
    {
        translation = vec4(translation.x, translation.y, translation.z + 1.0f, translation.w);
        translated = true;
    }

    if (m_rightPressed)
    {
        translation = vec4(translation.x + 1.0f, translation.y, translation.z, translation.w);
        translated = true;
    }

    if (!translated)
        return;

    if (length(translation) < MINIMUM_TRANSLATION_BIAS)
        return;

    vec4 transformedTranslation = rotationMatrix * translation;
    vec4 normalizedTranslation = normalize(transformedTranslation);

    m_position += vec3(normalizedTranslation.x, normalizedTranslation.y, normalizedTranslation.z) * deltaTime * CAMERA_MOVE_SPEED;

    UpdateViewMatrix();
}

mat4 Camera::GetViewMatrix()
{
    return m_viewMatrix;
}

mat4 Camera::GetRotationMatrix()
{
    mat4 rotationMatrix = mat4(1.0f);
    rotationMatrix = rotate(rotationMatrix, m_rotation.z, vec3(0.0f, 0.0f, 1.0f));
    rotationMatrix = rotate(rotationMatrix, m_rotation.y, vec3(0.0f, 1.0f, 0.0f));
    rotationMatrix = rotate(rotationMatrix, m_rotation.x, vec3(1.0f, 0.0f, 0.0f));

    return rotationMatrix;
}

void Camera::UpdateViewMatrix()
{
    vec4 forward = vec4(0.0f, 0.0f, -1.0f, 0.0f);

    mat4 rotationMatrix = GetRotationMatrix();
    forward = rotationMatrix * forward;

    vec3 forwardVec = vec3(forward.x, forward.y, forward.z);
    forwardVec = normalize(forwardVec);
    
    vec3 right = normalize(cross(forwardVec, vec3(0.0f, 1.0f, 0.0f)));
    vec3 up = normalize(cross(right, forwardVec));

    m_viewMatrix = lookAt(m_position, m_position + forwardVec, up);
}