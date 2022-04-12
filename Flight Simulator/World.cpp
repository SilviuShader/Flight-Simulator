#include <queue>
#include <unordered_set>
#include <glm/glm.hpp>

#include "World.h"
#include "InputWrapper.h"
#include "DebugHelper.h"
#include <glm/ext/matrix_transform.hpp>
#include "Biome.h"

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
	m_skybox  = new Skybox();
	m_terrain = new Terrain();

	m_worleyNoise = new WorleyNoise();
	m_worleyNoiseTexture = m_worleyNoise->RenderNoise({ 128, 3, 0.5f, 2, 4, 8, vec4(1.0f, 0.0f, 0.0f, 1.0f)});
	m_worleyNoise->RenderNoise({ 128, 3, 0.5f, 3, 5, 9, vec4(0.0f, 1.0f, 0.0f, 0.0f) }, m_worleyNoiseTexture);
	m_worleyNoise->RenderNoise({ 128, 3, 0.5f, 1, 2, 3, vec4(0.0f, 0.0f, 1.0f, 0.0f) }, m_worleyNoiseTexture);
}

World::~World()
{
	if (m_worleyNoiseTexture)
	{
		delete m_worleyNoiseTexture;
		m_worleyNoiseTexture = nullptr;
	}

	if (m_worleyNoise)
	{
		delete m_worleyNoise;
		m_worleyNoise = nullptr;
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

	if (InputWrapper::GetInstance()->GetKeyUp(InputWrapper::Keys::Debug))
		m_renderDebug = !m_renderDebug;

	if (m_renderDebug)
		DebugHelper::GetInstance()->ResetInstances();

	m_terrain->Udpate(m_camera, deltaTime, m_renderDebug);
}

float t = 0.0f;

void World::Draw()
{
	m_skybox->Draw(m_camera);

	m_terrain->Draw(m_camera, m_light);

	if (m_renderDebug)
		DebugHelper::GetInstance()->DrawRectangles(m_camera);

	DebugHelper::GetInstance()->DrawTexture3DSlice(m_worleyNoiseTexture, sinf(t) * 0.5f + 0.5f, 0.25f);

	t += 0.01f;
}

Camera* World::GetCamera() const
{
	return m_camera;
}