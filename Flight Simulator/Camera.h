#pragma once

#include <glm/glm.hpp>

class Camera
{
private:

    const float CAMERA_MOVE_SPEED        = 10.0f;
    const float CAMERA_ROTATE_SPEED      = 0.001f;
    const float MINIMUM_TRANSLATION_BIAS = 0.01f;

public:

    Camera(float, float, float, float, float);

    void      UpdateWindowSize(float, float);

    void      Update(float);
    
    glm::mat4 GetModelMatrix()      const;
    glm::mat4 GetViewMatrix()       const;
    glm::mat4 GetProjectionMatrix() const;
    glm::vec3 GetPosition()         const;

    float     GetWidth()            const;
    float     GetHeight()           const;
    float     GetAspectRatio()      const;
    float     GetFieldOfViewY()     const;

    float     GetNear()             const;
    float     GetFar()              const;

    glm::vec3 GetForward()          const;
    glm::vec3 GetRight()            const;
    glm::vec3 GetUp()               const;

private:

    glm::mat4 GetRotationMatrix();
    void      UpdateModelMatrix();
    void      UpdateViewMatrix();

private:

    float     m_fieldOfViewY;
    float     m_near;
    float     m_far;

    glm::vec3 m_position;
    glm::vec3 m_rotation;

    glm::vec3 m_forward;
    glm::vec3 m_right;
    glm::vec3 m_up;

    glm::mat4 m_modelMatrix;
    glm::mat4 m_viewMatrix;
    glm::mat4 m_projectionMatrix;

    float     m_width;
    float     m_height;
};