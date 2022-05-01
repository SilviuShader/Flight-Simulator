#include "Clouds.h"
#include "DebugHelper.h"
#include "ShaderManager.h"

using namespace glm;

Clouds::Clouds()
{
	m_worleyNoise = new WorleyNoise();
	m_perlinNoise = new PerlinNoise();

	m_worleyNoiseTexture = m_worleyNoise->RenderNoise({ 128, 3, 0.5f, 2, 4, 8, vec4(1.0f, 0.0f, 0.0f, 1.0f) });
	m_worleyNoise->RenderNoise({ 128, 3, 0.5f, 3, 5, 9, vec4(0.0f, 1.0f, 0.0f, 0.0f) }, m_worleyNoiseTexture);
	m_worleyNoise->RenderNoise({ 128, 3, 0.5f, 1, 2, 3, vec4(0.0f, 0.0f, 1.0f, 0.0f) }, m_worleyNoiseTexture);

	PerlinNoise::NoiseParameters noiseParameters;

	noiseParameters.StartPosition = vec2(-100.0f, -100.0f);
	noiseParameters.EndPosition = vec2(100.0f, 100.0f);
	noiseParameters.Exponent = 1;
	noiseParameters.FudgeFactor = 1;
	noiseParameters.OctavesCount = 4;
	noiseParameters.TextureSize = 1024;
	noiseParameters.Frequency = 0.005;

	m_weatherMap = m_perlinNoise->RenderSimplexNoise(noiseParameters, true);
}

Clouds::~Clouds()
{
	if (m_weatherMap)
	{
		delete m_weatherMap;
		m_weatherMap = nullptr;
	}

	if (m_worleyNoiseTexture)
	{
		delete m_worleyNoiseTexture;
		m_worleyNoiseTexture = nullptr;
	}

	if (m_perlinNoise)
	{
		delete m_perlinNoise;
		m_perlinNoise = nullptr;
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
	cloudsShader->SetTexture("WeatherMap",             m_weatherMap,         3);

	mat4 cameraMatrix = camera->GetModelMatrix();

	cloudsShader->SetMatrix4("CameraMatrix",      cameraMatrix);
	cloudsShader->SetFloat("AspectRatio",         camera->GetAspectRatio());
	cloudsShader->SetFloat("Near",                camera->GetNear());
	cloudsShader->SetFloat("Far",                 camera->GetFar());
	cloudsShader->SetFloat("FovY",                camera->GetFieldOfViewY());

	cloudsShader->SetVec3("BoundsMin",            vec3(-200.0f, -100.0f,  -200.0f));
	cloudsShader->SetVec3("BoundsMax",            vec3( 200.0f, 100.0f,  200.0f));
												  
	cloudsShader->SetVec3("CloudScale",           0.01f * vec3(1.0f, 1.0f, 1.0f));
	cloudsShader->SetVec3("CloudOffset",          vec3(0.0f, 0.0f, 0.0f));
	cloudsShader->SetFloat("DensityThreshold",    0.8f);
	cloudsShader->SetFloat("DensityMultiplier",   1.0f);
	cloudsShader->SetFloat("DarknessThreshold",   0.1f);
	cloudsShader->SetVec4("PhaseParams",          vec4(0.9f, 0.1f, 0.1f, 5.0f));
	cloudsShader->SetInt("FocusedEyeSunExponent", 2);
	cloudsShader->SetVec4("ShapeNoiseWeights",    vec4(1.0f, 0.5f, 0.25f, 0.0f));

	cloudsShader->SetVec4("DiffuseColor",         light->GetDiffuseColor());
	cloudsShader->SetVec3("LightDirection",       light->GetLightDirection());
												  
	cloudsShader->SetInt("LightStepsCount",       30);
												  
	cloudsShader->SetInt("StepsCount",            50);

	DebugHelper::GetInstance()->FullScreenQuadDrawCall();
}
