#include "glad/glad.h"
#include "RenderSettings.h"

using namespace glm;

RenderSettings* RenderSettings::g_instance = nullptr;

RenderSettings::~RenderSettings()
{
}

RenderSettings* RenderSettings::GetInstance()
{
	if (!g_instance)
		g_instance = new RenderSettings();

	return g_instance;
}

void RenderSettings::FreeInstance()
{
	if (g_instance)
	{
		delete g_instance;
		g_instance = nullptr;
	}
}

void RenderSettings::EnablePlaneClipping(vec4 plane)
{
	glEnable(GL_CLIP_DISTANCE0);
	m_clipPlane = plane;
	m_planeClippingEnabled = true;
}

void RenderSettings::DisablePlaneClipping()
{
	glDisable(GL_CLIP_DISTANCE0);
	m_planeClippingEnabled = false;
}

bool RenderSettings::PlaneClippingEnabled() const
{
	return m_planeClippingEnabled;
}

vec4 RenderSettings::ClipPlane() const
{
	return m_clipPlane;
}

RenderSettings::RenderSettings() :
	m_clipPlane(0.0f, 1.0f, 0.0f, 0.0f),
	m_planeClippingEnabled(false)
{
}
