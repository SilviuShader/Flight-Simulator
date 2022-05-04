#include "Clouds.h"
#include "DebugHelper.h"
#include "ShaderManager.h"
#include "Terrain.h"

using namespace glm;

Clouds::Clouds(CloudsProperties cloudsProperties)
{
	m_cloudsProperties = cloudsProperties;

	m_worleyNoise = new WorleyNoise();
	m_perlinNoise = new PerlinNoise();

	m_worleyNoiseTexture = m_worleyNoise->RenderNoise({ 128, 3, 0.5f, 2, 3, 4, vec4(1.0f, 0.0f, 0.0f, 1.0f) });
	m_worleyNoise->RenderNoise({ 128, 3, 0.5f, 3, 5, 9, vec4(0.0f, 1.0f, 0.0f, 0.0f) }, m_worleyNoiseTexture);
	m_worleyNoise->RenderNoise({ 128, 3, 0.5f, 1, 2, 3, vec4(0.0f, 0.0f, 1.0f, 0.0f) }, m_worleyNoiseTexture);

	m_detailNoiseTexture = m_worleyNoise->RenderNoise({ 32, 3, 0.5f, 2, 4, 8, vec4(1.0f, 0.0f, 0.0f, 1.0f) });
	m_worleyNoise->RenderNoise({ 32, 3, 0.5f, 3, 5, 9, vec4(0.0f, 1.0f, 0.0f, 0.0f) }, m_detailNoiseTexture);
	m_worleyNoise->RenderNoise({ 32, 3, 0.5f, 8, 9, 10, vec4(0.0f, 0.0f, 1.0f, 0.0f) }, m_detailNoiseTexture);

	PerlinNoise::NoiseParameters noiseParameters;

	noiseParameters.StartPosition = vec2(-100.0f, -100.0f);
	noiseParameters.EndPosition   = vec2(100.0f, 100.0f);
	noiseParameters.Exponent      = 2;
	noiseParameters.FudgeFactor   = 1.2;
	noiseParameters.OctavesCount  = 32;
	noiseParameters.TextureSize   = 1024;
	noiseParameters.Frequency     = 0.01;

	m_cloudsOffset  = vec3(0.0f, 0.0f, 0.0f);
	m_detailsOffset = vec3(0.0f, 0.0f, 0.0f);

	m_weatherMap = m_perlinNoise->RenderSimplexNoise(noiseParameters, true);
}

Clouds::~Clouds()
{
	if (m_weatherMap)
	{
		delete m_weatherMap;
		m_weatherMap = nullptr;
	}

	if (m_detailNoiseTexture)
	{
		delete m_detailNoiseTexture;
		m_detailNoiseTexture = nullptr;
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

void Clouds::Update(float deltaTime)
{
	UpdateOffset(m_cloudsOffset,  m_cloudsProperties.OffsetVelocity,        deltaTime);
	UpdateOffset(m_detailsOffset, m_cloudsProperties.DetailsOffsetVelocity, deltaTime);
}

void Clouds::Draw(Camera* camera, Light* light, Texture* sceneTexture, Texture* depthTexture)
{
	ShaderManager* shaderManager = ShaderManager::GetInstance();
	Shader*        cloudsShader  = shaderManager->GetCloudsShader();

	cloudsShader->Use();

	cloudsShader->SetTexture("SceneTexture",              sceneTexture,         0);
	cloudsShader->SetTexture("DepthTexture",              depthTexture,         1);
	cloudsShader->SetTexture3D("CloudsDensityTexture",    m_worleyNoiseTexture, 2);
	cloudsShader->SetTexture3D("DetailNoiseTexture",      m_detailNoiseTexture, 3);
	cloudsShader->SetTexture("WeatherMap",                m_weatherMap,         4);
													      
	mat4 cameraMatrix = camera->GetModelMatrix();	      
													      
	cloudsShader->SetMatrix4("CameraMatrix",              cameraMatrix);
	cloudsShader->SetFloat("AspectRatio",                 camera->GetAspectRatio());
	cloudsShader->SetFloat("Near",                        camera->GetNear());
	cloudsShader->SetFloat("Far",                         camera->GetFar());
	cloudsShader->SetFloat("FovY",                        camera->GetFieldOfViewY());

	vec3 position = camera->GetPosition();
	position = vec3(position.x, 0.0f, position.z);
	vec3 extent = vec3(m_cloudsProperties.CloudBoxExtents.x, 0.0f, m_cloudsProperties.CloudBoxExtents.y);

	vec3 boundsMin = position - extent + vec3(0.0f, Terrain::WATER_LEVEL,              0.0f);
	vec3 boundsMax = position + extent + vec3(0.0f, m_cloudsProperties.CloudsAltitude, 0.0f);
												          
	cloudsShader->SetVec3("BoundsMin",                    boundsMin);
	cloudsShader->SetVec3("BoundsMax",                    boundsMax);
												          
	cloudsShader->SetVec3("CloudScale",                   0.006f * vec3(1.0f, 1.0f, 1.0f));
	cloudsShader->SetFloat("DetailNoiseScale",            4.0f);
	cloudsShader->SetVec3("CloudOffset",                  m_cloudsOffset);
	cloudsShader->SetVec3("DetailsOffset",                m_detailsOffset);
	cloudsShader->SetFloat("DensityMultiplier",           0.1f * 0.82f);
	cloudsShader->SetFloat("DarknessThreshold",           0.28f);
	cloudsShader->SetFloat("DensityOffset",               -3.64 * 0.2f);
	cloudsShader->SetVec4("PhaseParams",                  vec4(0.72f, 0.33f, 1.0f, 0.83f));
	cloudsShader->SetInt("FocusedEyeSunExponent",         1);
	cloudsShader->SetVec4("ShapeNoiseWeights",            vec4(1.0f, 0.5f, 0.25f, 0.0f));
	cloudsShader->SetVec4("DetailNoiseWeights",           vec4(0.25f, 1.0f, 0.5f, 0.0f));
	cloudsShader->SetFloat("LightAbsorbtionTowardSun",    0.6);
	cloudsShader->SetFloat("LightAbsorptionThroughCloud", 1.05);
	cloudsShader->SetFloat("DetailNoiseWeight",           20.0f);
	cloudsShader->SetFloat("RayMarchStepSize",            11.0f);

	cloudsShader->SetVec4("DiffuseColor",                 light->GetDiffuseColor());
	cloudsShader->SetVec3("LightDirection",               light->GetLightDirection());
												          
	cloudsShader->SetInt("LightStepsCount",               10);
	
	DebugHelper::GetInstance()->FullScreenQuadDrawCall();
}

void Clouds::UpdateOffset(vec3& offset, vec3& offsetVelocity, float deltaTime)
{
	offset += offsetVelocity * deltaTime;

	if (offset.x >= 2.0f ||
		offset.y >= 2.0f ||
		offset.z >= 2.0f)
	{
		float x = offset.x;
		float y = offset.y;
		float z = offset.z;

		if (x >= 2.0f)
			x -= 2.0f;
		if (y >= 2.0f)
			y -= 2.0f;
		if (z >= 2.0f)
			z -= 2.0f;

		offset = vec3(x, y, z);
	}
}
