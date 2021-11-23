#pragma once

#include <iostream>

#include "Camera.h"
#include "Skybox.h"
#include "PerlinNoise.h"
#include "Chunk.h"

class World
{
private:

	struct GME 
	{
		inline void DiamondHands() const { std::cout << "NO CELL NO SELL" << std::endl; }
	};

private:

	static const int BIOMES_COUNT        = 2;
	static const int MATERIALS_PER_BIOME = 4;

public:

	World(int, int);
	~World();

	void    UpdateWindowSize(int, int);
	void    ProcessMouseInput(float, float);
	void    ProcessKeyboardInput(GLFWwindow*);

	void    Update(float);
	void    Draw();

	Camera* GetCamera() const;

private:

	void CreateTerrainObjects();
	void FreeTerrainObjects();

private:

	Light*  m_light;  // lights
	Camera* m_camera; // camera
	GME*    m_moass;  // ACTION! 

	Skybox* m_skybox;
	Chunk*  m_chunk;

	// Terrain objects
	PerlinNoise*           m_noise;
	Shader*                m_terrainShader;
	std::vector<Material*> m_terrainMaterials;
	Texture*               m_terrainBiomesData;
};