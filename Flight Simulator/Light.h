#pragma once

#include <glm/glm.hpp>

class Light
{
public:

    Light();

    void      SetAmbientColor(const glm::vec4&);
    void      SetDiffuseColor(const glm::vec4&);
    void      SetLightDirection(const glm::vec3&);
    void      SetSpecularPower(int);

    glm::vec4 GetAmbientColor()   const;
    glm::vec4 GetDiffuseColor()   const;
    glm::vec3 GetLightDirection() const;
    int       GetSpecularPower()  const;

private:

    glm::vec4 m_ambientColor;
    glm::vec4 m_diffuseColor;
    glm::vec3 m_lightDirection;
    float     m_specularPower;
};