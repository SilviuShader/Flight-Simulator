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

	Light*         m_light;
	Camera*        m_camera;
	Terrain*       m_terrain;
	Skybox*        m_skybox;
	Clouds*        m_clouds;

	RenderTexture* m_worldRenderTexture;

	WorleyNoise*   m_worleyNoise;
	Texture3D*     m_worleyNoiseTexture;
				   
	bool           m_renderDebug;
};