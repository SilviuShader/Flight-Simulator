#include "Clouds.h"
#include "DebugHelper.h"
#include "ShaderManager.h"

using namespace glm;

Clouds::Clouds()
{
	m_worleyNoise = new WorleyNoise();
	m_worleyNoiseTexture = m_worleyNoise->RenderNoise({ 128, 3, 0.5f, 2, 8, 16, vec4(1.0f, 0.0f, 0.0f, 1.0f) });
	m_worleyNoise->RenderNoise({ 128, 3, 0.5f, 3, 5, 9, vec4(0.0f, 1.0f, 0.0f, 0.0f) }, m_worleyNoiseTexture);
	m_worleyNoise->RenderNoise({ 128, 3, 0.5f, 1, 2, 3, vec4(0.0f, 0.0f, 1.0f, 0.0f) }, m_worleyNoiseTexture);
}

Clouds::~Clouds()
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
}

void Clouds::Draw(Camera* camera, Light* light, Texture* sceneTexture, Texture* depthTexture)
{
	ShaderManager* shaderManager = ShaderManager::GetInstance();
	Shader*        cloudsShader  = shaderManager->GetCloudsShader();

	cloudsShader->Use();

	cloudsShader->SetTexture("SceneTexture",           sceneTexture,         0);
	cloudsShader->SetTexture("DepthTexture",           depthTexture,         1);
	cloudsShader->SetTexture3D("CloudsDensityTexture", m_worleyNoiseTexture, 2);

	mat4 cameraMatrix = camera->GetModelMatrix();

	cloudsShader->SetMatrix4("CameraMatrix",      cameraMatrix);
	cloudsShader->SetFloat("AspectRatio",         camera->GetAspectRatio());
	cloudsShader->SetFloat("Near",                camera->GetNear());
	cloudsShader->SetFloat("Far",                 camera->GetFar());
	cloudsShader->SetFloat("FovY",                camera->GetFieldOfViewY());
												  
	cloudsShader->SetVec3("CloudScale",           0.01f * vec3(1.0f, 1.0f, 1.0f));
	cloudsShader->SetVec3("CloudOffset",          vec3(0.0f, 0.0f, 0.0f));
	cloudsShader->SetFloat("DensityThreshold",    0.8f);
	cloudsShader->SetFloat("DensityMultiplier",   2.0f);
	cloudsShader->SetFloat("DarknessThreshold",   0.1f);
	cloudsShader->SetVec4("PhaseParams",          vec4(0.9f, 0.1f, 0.1f, 5.0f));
	cloudsShader->SetInt("FocusedEyeSunExponent", 2);

	cloudsShader->SetVec4("DiffuseColor",         light->GetDiffuseColor());
	cloudsShader->SetVec3("LightDirection",       light->GetLightDirection());
												  
	cloudsShader->SetInt("LightStepsCount",       10);
												  
	cloudsShader->SetInt("StepsCount",            30);

	DebugHelper::GetInstance()->FullScreenQuadDrawCall();
}
