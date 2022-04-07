#include <queue>
#include <unordered_set>
#include <glm/glm.hpp>

#include "World.h"
#include "InputWrapper.h"
#include "Shapes.h"
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
}

World::~World()
{
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
		Shapes::GetInstance()->ResetInstances();

	m_terrain->Udpate(m_camera, deltaTime, m_renderDebug);
}

void World::Draw()
{
	m_skybox->Draw(m_camera);

	m_terrain->Draw(m_camera, m_light);

	if (m_renderDebug)
		Shapes::GetInstance()->DrawRectangles(m_camera);
}

Camera* World::GetCamera() const
{
	return m_camera;
}