#pragma once

#include <glm/glm.hpp>

class RenderSettings
{
public:

	RenderSettings(const RenderSettings&) = delete;
	void operator=(const RenderSettings&) = delete;

	~RenderSettings();

	static RenderSettings* GetInstance();
	static void            FreeInstance();

	       void            EnablePlaneClipping(glm::vec4);
		   void            DisablePlaneClipping();

		   bool            PlaneClippingEnabled() const;
		   glm::vec4       ClipPlane()            const;

private:

	RenderSettings();

private:

	       bool            m_planeClippingEnabled;
		   glm::vec4       m_clipPlane;

	static RenderSettings* g_instance;
};