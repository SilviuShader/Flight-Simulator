#include "Clouds.h"
#include "DebugHelper.h"
#include "ShaderManager.h"

using namespace glm;

Clouds::Clouds()
{
}

void Clouds::Draw(Camera* camera, Texture* sceneTexture, Texture* depthTexture)
{
	ShaderManager* shaderManager = ShaderManager::GetInstance();
	Shader*        cloudsShader  = shaderManager->GetCloudsShader();

	cloudsShader->Use();

	cloudsShader->SetTexture("SceneTexture", sceneTexture, 0);
	cloudsShader->SetTexture("DepthTexture", depthTexture, 1);

	mat4 cameraMatrix = camera->GetModelMatrix();

	cloudsShader->SetMatrix4("CameraMatrix", cameraMatrix);
	cloudsShader->SetFloat("AspectRatio",    camera->GetAspectRatio());
	cloudsShader->SetFloat("Near",           camera->GetNear());
	cloudsShader->SetFloat("Far",            camera->GetFar());
	cloudsShader->SetFloat("FovY",           camera->GetFieldOfViewY());

	DebugHelper::GetInstance()->FullScreenQuadDrawCall();
}
