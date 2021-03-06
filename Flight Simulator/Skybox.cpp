#include "glad/glad.h"
#include "Skybox.h"

#include <glm/gtc/matrix_transform.hpp>
#include "VertexTypes.h"
#include "ShaderManager.h"

using namespace glm;

Skybox::Skybox()
{
	CreateCubeBuffers();

	m_cubemap = new Cubemap(
	{
		"Assets/Skybox/right.jpg",
		"Assets/Skybox/left.jpg",
		"Assets/Skybox/top.jpg",
		"Assets/Skybox/bottom.jpg",
		"Assets/Skybox/front.jpg",
		"Assets/Skybox/back.jpg"
	});
}

Skybox::~Skybox()
{
	if (m_cubemap)
	{
		delete m_cubemap;
		m_cubemap = nullptr;
	}

	FreeCubeBuffers();
}

void Skybox::Draw(Camera* camera)
{
	mat4           model         = translate(mat4(1.0f), camera->GetPosition()) * rotate(mat4(1.0f), 0.0f, vec3(1.0f, 0.0f, 0.0f)) * scale(mat4(1.0f), vec3(500.0f, 500.0f, 500.0f));
	mat4           view          = camera->GetViewMatrix();
	mat4           projection    = camera->GetProjectionMatrix();

	ShaderManager* shaderManager = ShaderManager::GetInstance();

	Shader*        skyboxShader  = shaderManager->GetSkyboxShader();

	skyboxShader->Use();

	skyboxShader->SetMatrix4("Model", model);
	skyboxShader->SetMatrix4("View", view);
	skyboxShader->SetMatrix4("Projection", projection);

	skyboxShader->SetCubemap("Skybox", m_cubemap, 0);

	glBindVertexArray(m_vao);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
}

void Skybox::CreateCubeBuffers()
{
	int verticesCount = 8;

	VertexPosition cubeVertices[] =
	{
		// bottom vertices
		vec3(-1.0f, -1.0f, 1.0f),  // front-left
		vec3(1.0f, -1.0f, 1.0f),   // front-right
		vec3(1.0f, -1.0f, -1.0f),  // back-right
		vec3(-1.0f, -1.0f, -1.0f), // back-left

		// top vertices
		vec3(-1.0f, 1.0f, 1.0f),   // front-left
		vec3(1.0f, 1.0f, 1.0f),    // front-right
		vec3(1.0f, 1.0f, -1.0f),   // back-right
		vec3(-1.0f, 1.0f, -1.0f)   // back-left
	};

	// counter clockwise
	unsigned int indices[] =
	{
		// bottom face
		0, 1, 3,  
		1, 2, 3,

		// top face
		4, 7, 5,
		5, 7, 6,

		// left face
		0, 3, 4,
		3, 7, 4,

		// right face
		1, 5, 2,
		2, 5, 6,

		// back face
		3, 2, 7,
		2, 6, 7,

		// front face
		0, 4, 1,
		1, 4, 5
	};

	glGenVertexArrays(1, &m_vao);

	glBindVertexArray(m_vao);

	glGenBuffers(1, &m_vbo);

	glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(VertexPosition) * verticesCount, cubeVertices, GL_STATIC_DRAW);

	VertexPosition::SetLayout();

	glGenBuffers(1, &m_ebo);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * 36, indices, GL_STATIC_DRAW);
}

void Skybox::FreeCubeBuffers()
{
	glBindVertexArray(m_vao);

	VertexPosition::ResetLayout();

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glDeleteBuffers(1, &m_vbo);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glDeleteBuffers(1, &m_ebo);

	glBindVertexArray(0);
	glDeleteVertexArrays(1, &m_vao);
}

