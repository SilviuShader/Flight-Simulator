#include "glad/glad.h"
#include "DebugHelper.h"
#include <glm/ext/matrix_transform.hpp>
#include "VertexTypes.h"
#include "ShaderManager.h"

using namespace glm;

DebugHelper* DebugHelper::g_instance = nullptr;

DebugHelper::~DebugHelper()
{
	FreeQuadBuffers();
	FreeCubeBuffers();
}

DebugHelper* DebugHelper::GetInstance()
{
	if (!g_instance)
		g_instance = new DebugHelper();

	return g_instance;
}

void DebugHelper::FreeInstance()
{
	if (g_instance)
	{
		delete g_instance;
		g_instance = nullptr;
	}
}

void DebugHelper::ResetInstances()
{
	m_rectangleInstances.clear();
}

void DebugHelper::AddRectangleInstance(const vec3& center, const vec3& extents)
{
	m_rectangleInstances.push_back(translate(mat4(1.0), center) * scale(mat4(1.0), extents));
}

void DebugHelper::DrawRectangles(Camera* camera)
{
	ShaderManager* shaderManager = ShaderManager::GetInstance();
	Shader*        colorShader   = shaderManager->GetColorShader();

	glBindBuffer(GL_ARRAY_BUFFER, m_cubeInstanceVbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(mat4) * m_rectangleInstances.size(), ((m_rectangleInstances.size()) ? &m_rectangleInstances[0] : NULL), GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	
	mat4 view       = camera->GetViewMatrix();
	mat4 projection = camera->GetProjectionMatrix();

	colorShader->Use();

	colorShader->SetMatrix4("View",       view);
	colorShader->SetMatrix4("Projection", projection);

	glLineWidth(1.0);

	glBindVertexArray(m_cubeVAO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_cubeEBO);
	glDrawElementsInstanced(GL_LINES, 24, GL_UNSIGNED_INT, 0, m_rectangleInstances.size());
}

void DebugHelper::DrawTexture3DSlice(Texture3D* texture3D, float slice, float scale)
{
	glDisable(GL_DEPTH);

	ShaderManager* shaderManager        = ShaderManager::GetInstance();
	Shader*        texture3DSliceShader = shaderManager->GetTexture3DSliceShader();

	texture3DSliceShader->Use();

	texture3DSliceShader->SetFloat("Scale",     scale);
	
	texture3DSliceShader->SetTexture3D("Image", texture3D, 0);
	texture3DSliceShader->SetFloat("Slice",     slice);

	glBindVertexArray(m_quadVAO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER_BINDING, m_quadEBO);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

	glEnable(GL_DEPTH);
}

DebugHelper::DebugHelper()
{
	CreateCubeBuffers();
	CreateQuadBuffers();
}

void DebugHelper::CreateCubeBuffers()
{
	VertexPositionColor vertices[] =
	{
		VertexPositionColor(vec3(-1.0f, -1.0f,  1.0f), vec3(0.0f, 1.0f, 0.0f)), // left-bottom-front
		VertexPositionColor(vec3( 1.0f, -1.0f,  1.0f), vec3(0.0f, 1.0f, 0.0f)), // right-bottom-front
		VertexPositionColor(vec3( 1.0f, -1.0f, -1.0f), vec3(0.0f, 1.0f, 0.0f)), // right-bottom-back
		VertexPositionColor(vec3(-1.0f, -1.0f, -1.0f), vec3(0.0f, 1.0f, 0.0f)), // left-bottom-back

		VertexPositionColor(vec3(-1.0f,  1.0f,  1.0f), vec3(0.0f, 1.0f, 0.0f)), // left-top-front
		VertexPositionColor(vec3( 1.0f,  1.0f,  1.0f), vec3(0.0f, 1.0f, 0.0f)), // right-top-front
		VertexPositionColor(vec3( 1.0f,  1.0f, -1.0f), vec3(0.0f, 1.0f, 0.0f)), // right-top-back
		VertexPositionColor(vec3(-1.0f,  1.0f, -1.0f), vec3(0.0f, 1.0f, 0.0f))  // left-top-back
	};

	unsigned int indices[] =
	{
		// bottom face
		0, 1, 
		1, 2,
		2, 3,
		3, 0,

		// vertical lines
		0, 4,
		1, 5,
		2, 6,
		3, 7,

		// top face
		4, 5,
		5, 6,
		6, 7,
		7, 4
	};

	glGenVertexArrays(1, &m_cubeVAO);
	glBindVertexArray(m_cubeVAO);

	glGenBuffers(1, &m_cubeVBO);

	glBindBuffer(GL_ARRAY_BUFFER, m_cubeVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	VertexPositionColor::SetLayout();

	glGenBuffers(1, &m_cubeInstanceVbo);
	glBindBuffer(GL_ARRAY_BUFFER, m_cubeInstanceVbo);
	glBufferData(GL_ARRAY_BUFFER, 0, NULL, GL_DYNAMIC_DRAW);

	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(vec4), (void*)0);
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(vec4), (void*)(sizeof(vec4)));
	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(vec4), (void*)(sizeof(vec4) * 2));
	glEnableVertexAttribArray(5);
	glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(vec4), (void*)(sizeof(vec4) * 3));

	glVertexAttribDivisor(2, 1);
	glVertexAttribDivisor(3, 1);
	glVertexAttribDivisor(4, 1);
	glVertexAttribDivisor(5, 1);

	glGenBuffers(1, &m_cubeEBO);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_cubeEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
}

void DebugHelper::FreeCubeBuffers()
{
	glBindVertexArray(m_cubeVAO);

	VertexPositionColor::ResetLayout();

	glDisableVertexAttribArray(2);
	glDisableVertexAttribArray(3);
	glDisableVertexAttribArray(4);
	glDisableVertexAttribArray(5);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glDeleteBuffers(1, &m_cubeVBO);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glDeleteBuffers(1, &m_cubeInstanceVbo);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glDeleteBuffers(1, &m_cubeEBO);

	glBindVertexArray(0);
	glDeleteVertexArrays(1, &m_cubeVAO);
}

void DebugHelper::CreateQuadBuffers()
{
	VertexPositionTexture vertices[] =
	{
		VertexPositionTexture(vec3(-1.0f, -1.0f, 0.0f), vec2(0.0f, 0.0f)),
		VertexPositionTexture(vec3( 1.0f, -1.0f, 0.0f), vec2(1.0f, 0.0f)),
		VertexPositionTexture(vec3( 1.0f,  1.0f, 0.0f), vec2(1.0f, 1.0f)),
		VertexPositionTexture(vec3(-1.0f,  1.0f, 0.0f), vec2(0.0f, 1.0f))
	};

	unsigned int indices[] =
	{
		0, 1, 2,
		0, 2, 3
	};

	glGenVertexArrays(1, &m_quadVAO);
	glBindVertexArray(m_quadVAO);

	glGenBuffers(1, &m_quadVBO);

	glBindBuffer(GL_ARRAY_BUFFER, m_quadVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	VertexPositionTexture::SetLayout();

	glGenBuffers(1, &m_quadEBO);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_quadEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
}

void DebugHelper::FreeQuadBuffers()
{
	glBindVertexArray(m_quadVAO);

	VertexPositionTexture::ResetLayout();

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glDeleteBuffers(1, &m_quadVBO);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glDeleteBuffers(1, &m_quadEBO);

	glBindVertexArray(0);
	glDeleteVertexArrays(1, &m_quadVAO);
}
