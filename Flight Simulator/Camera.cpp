#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "glad/glad.h"

#include "Camera.h"

using namespace glm;

Camera::Camera(float fieldOfViewY, float width, float height, float near, float far) :
    m_position(vec3(0.0f, 10.0f, 0.0f)),
    m_rotation(vec3(0.0f, 0.0f, 0.0f)),
    m_width(width),
    m_height(height),
    m_upPressed(false),
    m_leftPressed(false),
    m_downPressed(false),
    m_rightPressed(false),
    m_fieldOfViewY(fieldOfViewY),
    m_near(near),
    m_far(far),
    m_forward(vec3(0.0f, 0.0f, -1.0f)),
    m_right(vec3(1.0f, 0.0f, 0.0f)),
    m_up(vec3(0.0f, 1.0f, 0.0f))
{
    m_projectionMatrix = perspective(fieldOfViewY, width / height, near, far);
    UpdateViewMatrix();
}

void Camera::ProcessKeybaordInput(GLFWwindow* window)
{
    m_upPressed    = glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS;
    m_leftPressed  = glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS;
    m_downPressed  = glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS;
    m_rightPressed = glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS;
}

void Camera::ProcessMouseInput(float diffX, float diffY)
{
    m_rotation.x += diffY * CAMERA_ROTATE_SPEED;
    m_rotation.y += diffX * CAMERA_ROTATE_SPEED;

    constexpr float halfPi = half_pi<float>();

    if (m_rotation.x >= halfPi)
        m_rotation.x = halfPi;
    if (m_rotation.x <= -halfPi)
        m_rotation.x = -halfPi;

    UpdateViewMatrix();
}

void Camera::UpdateWindowSize(float width, float height)
{
    m_width = width;
    m_height = height;
    m_projectionMatrix = perspective(m_fieldOfViewY, width / height, m_near, m_far);
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

    vec4 transformedTranslation = translation * rotationMatrix;
    vec4 normalizedTranslation = normalize(transformedTranslation);

    m_position += vec3(normalizedTranslation.x, normalizedTranslation.y, normalizedTranslation.z) * deltaTime * CAMERA_MOVE_SPEED;

    UpdateViewMatrix();
}

mat4 Camera::GetViewMatrix() const
{
    return m_viewMatrix;
}

mat4 Camera::GetProjectionMatrix() const
{
    return m_projectionMatrix;
}

vec3 Camera::GetPosition() const
{
    return m_position;
}

float Camera::GetWidth() const
{
    return m_width;
}

float Camera::GetHeight() const
{
    return m_height;
}

float Camera::GetAspectRatio() const
{
    return m_width / m_height;
}

float Camera::GetFieldOfViewY() const
{
    return m_fieldOfViewY;
}

float Camera::GetNear() const
{
    return m_near;
}

float Camera::GetFar() const
{
    return m_far;
}

glm::vec3 Camera::GetForward() const
{
    return m_forward;
}

glm::vec3 Camera::GetRight() const
{
    return m_right;
}

glm::vec3 Camera::GetUp() const
{
    return m_up;
}

mat4 Camera::GetRotationMatrix()
{
    mat4 rotationMatrix = mat4(1.0f);

    rotationMatrix = rotate(rotationMatrix, m_rotation.x, vec3(1.0f, 0.0f, 0.0f));
    rotationMatrix = rotate(rotationMatrix, m_rotation.y, vec3(0.0f, 1.0f, 0.0f));
    rotationMatrix = rotate(rotationMatrix, m_rotation.z, vec3(0.0f, 0.0f, 1.0f));
    
    return rotationMatrix;
}

void Camera::UpdateViewMatrix()
{
    vec4 forward = vec4(0.0f, 0.0f, -1.0f, 0.0f);
    vec4 right   = vec4(1.0f, 0.0f, 0.0f,  0.0f);
    vec4 up      = vec4(0.0f, 1.0f, 0.0f,  0.0f);

    mat4 rotationMatrix = GetRotationMatrix();
    
    forward = forward * rotationMatrix;
    right   = right   * rotationMatrix;
    up      = up      * rotationMatrix;

    m_forward = normalize(vec3(forward.x, forward.y, forward.z));
    m_right   = normalize(vec3(right.x,   right.y,   right.z));
    m_up      = normalize(vec3(up.x,      up.y,      up.z));
    
    m_viewMatrix = lookAt(m_position, m_position + m_forward, m_up);
}
