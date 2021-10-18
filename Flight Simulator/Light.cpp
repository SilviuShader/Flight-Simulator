#include "Light.h"

using namespace glm;

Light::Light() :
    m_ambientColor(vec4(0.25f, 0.25f, 0.25f, 1.0f)),
    m_diffuseColor(vec4(1.0f, 1.0f, 1.0f, 1.0f)),
    m_lightDirection(vec3(1.0f, -1.0f, 0.0f))
{
}

void Light::SetAmbientColor(const vec4& ambientColor)
{
    m_ambientColor = ambientColor;
}

void Light::SetDiffuseColor(const vec4& diffuseColor)
{
    m_diffuseColor = diffuseColor;
}

void Light::SetLightDirection(const vec3& lightDirection)
{
    m_lightDirection = lightDirection;
}

vec4 Light::GetAmbientColor() const
{
    return m_ambientColor;
}

vec4 Light::GetDiffuseColor() const
{
    return m_diffuseColor;
}

vec3 Light::GetLightDirection() const
{
    return m_lightDirection;
}