#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "glad/glad.h"

#include "Camera.h"
#include "InputWrapper.h"

using namespace std;
using namespace glm;

Camera::Camera()
{
}

Camera::Camera(float fieldOfViewY, float width, float height, float near, float far) :
    m_position(vec3(0.0f, 60.0f, 0.0f)),
    m_rotation(vec3(half_pi<float>() / 4.0f, 0.0f, 0.0f)),
    m_width(width),
    m_height(height),
    m_fieldOfViewY(fieldOfViewY),
    m_near(near),
    m_far(far)
{
    m_directions.Forward = vec3(0.0f, 0.0f, -1.0f);
    m_directions.Right   = vec3(1.0f, 0.0f, 0.0f);
    m_directions.Up      = vec3(0.0f, 1.0f, 0.0f);

    m_projectionMatrix = perspective(fieldOfViewY, width / height, near, far);
    UpdateViewMatrix();
    UpdateModelMatrix();
}

void Camera::UpdateWindowSize(float width, float height)
{
    m_width = width;
    m_height = height;
    m_projectionMatrix = perspective(m_fieldOfViewY, width / height, m_near, m_far);
}

void Camera::Update(float deltaTime)
{
    auto mouseDiff = InputWrapper::GetInstance()->GetMouseMoveDiff();

    m_rotation.x += mouseDiff.y * CAMERA_ROTATE_SPEED;
    m_rotation.y += mouseDiff.x * CAMERA_ROTATE_SPEED;

    constexpr float halfPi = half_pi<float>();

    if (m_rotation.x >= halfPi)
        m_rotation.x = halfPi;

    if (m_rotation.x <= -halfPi)
        m_rotation.x = -halfPi;

    UpdateViewMatrix();
    UpdateModelMatrix();

    mat4 rotationMatrix = GetRotationMatrix();

    vec4 translation    = vec4(0.0f, 0.0f, 0.0f, 0.0f);
    bool translated     = false;

    if (InputWrapper::GetInstance()->GetKey(InputWrapper::Keys::Up))
    {
        translation = vec4(translation.x, translation.y, translation.z - 1.0f, translation.w);
        translated = true;
    }

    if (InputWrapper::GetInstance()->GetKey(InputWrapper::Keys::Left))
    {
        translation = vec4(translation.x - 1.0f, translation.y, translation.z, translation.w);
        translated = true;
    }

    if (InputWrapper::GetInstance()->GetKey(InputWrapper::Keys::Down))
    {
        translation = vec4(translation.x, translation.y, translation.z + 1.0f, translation.w);
        translated = true;
    }

    if (InputWrapper::GetInstance()->GetKey(InputWrapper::Keys::Right))
    {
        translation = vec4(translation.x + 1.0f, translation.y, translation.z, translation.w);
        translated = true;
    }

    if (!translated)
        return;

    if (length(translation) < MINIMUM_TRANSLATION_BIAS)
        return;

    vec4 transformedTranslation = translation * rotationMatrix;
    vec4 normalizedTranslation  = normalize(transformedTranslation);

    m_position += vec3(normalizedTranslation.x, normalizedTranslation.y, normalizedTranslation.z) * deltaTime * CAMERA_MOVE_SPEED;

    UpdateViewMatrix();
    UpdateModelMatrix();
}

mat4 Camera::GetModelMatrix() const
{
    return m_modelMatrix;
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

vec3 Camera::GetForward() const
{
    return m_directions.Forward;
}

vec3 Camera::GetRight() const
{
    return m_directions.Right;
}

vec3 Camera::GetUp() const
{
    return m_directions.Up;
}

Camera::Directions Camera::GetReflectedVectors()
{
    vec4 forward = vec4(0.0f, 0.0f, -1.0f, 0.0f);
    vec4 right   = vec4(1.0f, 0.0f, 0.0f, 0.0f);
    vec4 up      = vec4(0.0f, 1.0f, 0.0f, 0.0f);

    m_rotation.x = -m_rotation.x;

    mat4 rotationMatrix = GetRotationMatrix();

    forward = forward * rotationMatrix;
    right   = right   * rotationMatrix;
    up      = up      * rotationMatrix;
    
    m_rotation.x = -m_rotation.x;

    Directions result;

    result.Forward = vec3(forward.x, forward.y, forward.z);
    result.Right   = vec3(right.x,   right.y,   right.z);
    result.Up      = vec3(up.x,      up.y,      up.z);

    return result;
}

mat4 Camera::GetRotationMatrix()
{
    mat4 rotationMatrix = mat4(1.0f);

    rotationMatrix = rotate(rotationMatrix, m_rotation.x, vec3(1.0f, 0.0f, 0.0f));
    rotationMatrix = rotate(rotationMatrix, m_rotation.y, vec3(0.0f, 1.0f, 0.0f));
    rotationMatrix = rotate(rotationMatrix, m_rotation.z, vec3(0.0f, 0.0f, 1.0f));
    
    return rotationMatrix;
}

void Camera::UpdateModelMatrix()
{
    m_modelMatrix = inverse(m_viewMatrix);
}

void Camera::UpdateViewMatrix()
{
    vec4 forward        = vec4(0.0f, 0.0f, -1.0f, 0.0f);
    vec4 right          = vec4(1.0f, 0.0f, 0.0f,  0.0f);
    vec4 up             = vec4(0.0f, 1.0f, 0.0f,  0.0f);

    mat4 rotationMatrix = GetRotationMatrix();
    
    forward = forward * rotationMatrix;
    right   = right   * rotationMatrix;
    up      = up      * rotationMatrix;

    m_directions.Forward = normalize(vec3(forward.x, forward.y, forward.z));
    m_directions.Right   = normalize(vec3(right.x,   right.y,   right.z  ));
    m_directions.Up      = normalize(vec3(up.x,      up.y,      up.z     ));
    
    m_viewMatrix = lookAt(m_position, m_position + m_directions.Forward, m_directions.Up);
}
