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

	Light*       m_light;  // lights
	Camera*      m_camera; // camera
	GME*         m_moass;  // ACTION! 

	Skybox*      m_skybox;
	PerlinNoise* m_noise;
	Chunk*       m_chunk;
};