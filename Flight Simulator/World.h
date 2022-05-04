#pragma once

#include <iostream>
#include <bitset>
#include <unordered_map>

#include "Skybox.h"
#include "PerlinNoise.h"
#include "Chunk.h"
#include "Camera.h"
#include "Utils.h"
#include "Model.h"

#include "Terrain.h"
#include "WorleyNoise.h"
#include "Clouds.h"
#include "ReflectionCamera.h"

class World
{
public:

	World(int, int);
	~World();

	void    UpdateWindowSize(int, int);

	void    Update(float);
	void    Draw();

	Camera* GetCamera() const;

private:

	void RenderScene(Camera*, bool, RenderTexture* = nullptr, Texture* = nullptr, Texture* = nullptr);

private:

	Light*            m_light;
	Camera*           m_camera;
	ReflectionCamera* m_reflectionCamera;
	Terrain*          m_terrain;
	Skybox*           m_skybox;
	Clouds*           m_clouds;
				      
	RenderTexture*    m_auxilliaryRenderTexture;
	RenderTexture*    m_reflectionRenderTexture;
	RenderTexture*    m_refractionRenderTexture;
				      
	bool              m_renderDebug;
};