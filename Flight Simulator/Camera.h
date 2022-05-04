#pragma once

#include <utility>
#include <glm/glm.hpp>
#include "MathHelper.h"

class Camera
{
public:

    struct Directions
    {
    public:

        glm::vec3 Forward;
        glm::vec3 Right;
        glm::vec3 Up;
    };

private:

    const float CAMERA_MOVE_SPEED        = 10.0f;
    const float CAMERA_ROTATE_SPEED      = 0.001f;
    const float MINIMUM_TRANSLATION_BIAS = 0.01f;

public:

    Camera();
    Camera(float, float, float, float, float);

    virtual void       UpdateWindowSize(float, float);
                       
    virtual void       Update(float);
                       
    virtual glm::mat4  GetModelMatrix()                       const;
    virtual glm::mat4  GetViewMatrix()                        const;
    virtual glm::mat4  GetProjectionMatrix()                  const;
    virtual glm::vec3  GetPosition()                          const;
                                                      
    virtual float      GetWidth()                             const;
    virtual float      GetHeight()                            const;
    virtual float      GetAspectRatio()                       const;
    virtual float      GetFieldOfViewY()                      const;
                                                      
    virtual float      GetNear()                              const;
    virtual float      GetFar()                               const;
                                                      
    virtual glm::vec3  GetForward()                           const;
    virtual glm::vec3  GetRight()                             const;
    virtual glm::vec3  GetUp()                                const;

            Directions GetReflectedVectors();

private:

    glm::mat4 GetRotationMatrix();
    void      UpdateModelMatrix();
    void      UpdateViewMatrix();
    
private:

    float      m_fieldOfViewY;
    float      m_near;
    float      m_far;
               
    glm::vec3  m_position;
    glm::vec3  m_rotation;

    Directions m_directions;

    glm::mat4  m_modelMatrix;
    glm::mat4  m_viewMatrix;
    glm::mat4  m_projectionMatrix;
               
    float      m_width;
    float      m_height;
};