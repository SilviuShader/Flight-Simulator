#include <glm/glm.hpp>

#include "World.h"

using namespace glm;

World::World(int windowWidth, int windowHeight) :
	m_moass(nullptr)
{
	m_light = new Light();
	m_light->SetAmbientColor(vec4(0.75f, 0.75f, 0.75f, 1.0f));
	m_light->SetDiffuseColor(vec4(0.8f, 0.8f, 0.9f, 1.0f));

	m_camera = new Camera(radians(45.0f), (float)windowWidth, (float)windowHeight, 0.1f, 1000.0f);
	m_skybox = new Skybox();
	m_noise  = new PerlinNoise();
	m_chunk  = new Chunk(m_noise);
}

World::~World()
{
	if (m_chunk)
	{
		delete m_chunk;
		m_chunk = nullptr;
	}

	if (m_noise)
	{
		delete m_noise;
		m_noise = nullptr;
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

void World::ProcessMouseInput(float diffX, float diffY)
{
	m_camera->ProcessMouseInput(diffX, diffY);
}

void World::ProcessKeyboardInput(GLFWwindow* window)
{
	m_camera->ProcessKeybaordInput(window);
}

void World::Update(float deltaTime)
{
	m_camera->Update(deltaTime);
}

void World::Draw()
{
	m_skybox->Draw(m_camera);
	m_chunk->Draw(m_light, m_camera);
}

Camera* World::GetCamera() const
{
	return m_camera;
}
