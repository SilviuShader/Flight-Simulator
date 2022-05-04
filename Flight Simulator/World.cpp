#include <queue>
#include <unordered_set>
#include "glad/glad.h"
#include <glm/glm.hpp>

#include "World.h"
#include "InputWrapper.h"
#include "DebugHelper.h"
#include <glm/ext/matrix_transform.hpp>
#include "Biome.h"
#include "RenderSettings.h"

using namespace std;
using namespace glm;

World::World(int windowWidth, int windowHeight) :
	m_renderDebug(false)
{
	m_light = new Light();
	m_light->SetAmbientColor(vec4(0.2f, 0.2f, 0.2f, 1.0f));
	m_light->SetDiffuseColor(vec4(1.0f, 1.0f, 1.0f, 1.0f));
	m_light->SetSpecularPower(4.0f);

	m_camera  = new Camera(radians(45.0f), (float)windowWidth, (float)windowHeight, 0.1f, 1000.0f);
	m_reflectionCamera = new ReflectionCamera(m_camera, Terrain::WATER_LEVEL);
	m_skybox  = new Skybox();
	m_terrain = new Terrain();

	m_auxilliaryRenderTexture = new RenderTexture(windowWidth, windowHeight);
	m_reflectionRenderTexture = new RenderTexture(windowWidth, windowHeight);

	Clouds::CloudsProperties cloudsProperties;
	cloudsProperties.OffsetVelocity = vec3(.01f, .02f, .03f);
	cloudsProperties.DetailsOffsetVelocity = vec3(0.4f, 0.5f, 0.6f);
	cloudsProperties.CloudBoxExtents = vec2(200.0f, 200.0f);
	cloudsProperties.CloudsAltitude = 100.0f;

	m_clouds = new Clouds(cloudsProperties);
}

World::~World()
{
	if (m_clouds)
	{
		delete m_clouds;
		m_clouds = nullptr;
	}

	if (m_reflectionRenderTexture)
	{
		delete m_reflectionRenderTexture;
		m_reflectionRenderTexture = nullptr;
	}

	if (m_auxilliaryRenderTexture)
	{
		delete m_auxilliaryRenderTexture;
		m_auxilliaryRenderTexture = nullptr;
	}

	if (m_terrain)
	{
		delete m_terrain;
		m_terrain = nullptr;
	}

	if (m_skybox)
	{
		delete m_skybox;
		m_skybox = nullptr;
	}

	if (m_reflectionCamera)
	{
		delete m_reflectionCamera;
		m_reflectionCamera = nullptr;
	}

	if (m_camera)
	{
		delete m_camera;
		m_camera = nullptr;
	}

	if (m_light)
	{
		delete m_light;
		m_light = nullptr;
	}
}

void World::UpdateWindowSize(int width, int height)
{
	m_camera->UpdateWindowSize(width, height);
}

void World::Update(float deltaTime)
{
	m_camera->Update(deltaTime);
	m_reflectionCamera->Update(deltaTime);
	m_clouds->Update(deltaTime);

	if (InputWrapper::GetInstance()->GetKeyUp(InputWrapper::Keys::Debug))
		m_renderDebug = !m_renderDebug;

	if (m_renderDebug)
		DebugHelper::GetInstance()->ResetInstances();

	RenderSettings* renderSettings = RenderSettings::GetInstance();
	m_terrain->Udpate(m_reflectionCamera, 0.0f, m_renderDebug);

	renderSettings->EnablePlaneClipping(vec4(0.0f, 1.0f, 0.0f, -Terrain::WATER_LEVEL));
	RenderScene(m_reflectionCamera, m_reflectionRenderTexture);
	renderSettings->DisablePlaneClipping();

	m_terrain->Udpate(m_camera, deltaTime, m_renderDebug);
}

void World::Draw()
{
	RenderScene(m_camera, nullptr, m_reflectionRenderTexture->GetTexture(), m_reflectionCamera);
	//DebugHelper::GetInstance()->DrawFullscreenTexture(m_reflectionRenderTexture->GetTexture());
}

Camera* World::GetCamera() const
{
	return m_camera;
}

void World::RenderScene(Camera* camera, RenderTexture* targetTexture, Texture* reflectionTexture, Camera* reflectionCamera)
{
	m_auxilliaryRenderTexture->Begin();

	m_skybox->Draw(camera);

	m_terrain->Draw(camera, m_light, reflectionTexture, reflectionCamera);

	if (m_renderDebug)
		DebugHelper::GetInstance()->DrawRectangles(camera);

	if (targetTexture)
		targetTexture->Begin();
	else
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

	m_clouds->Draw(camera, m_light, m_auxilliaryRenderTexture->GetTexture(), m_auxilliaryRenderTexture->GetDepthTexture());
}
