#include "Clouds.h"
#include "DebugHelper.h"
#include "ShaderManager.h"

Clouds::Clouds()
{
}

void Clouds::Draw(Texture* sceneTexture, Texture* depthTexture)
{
	ShaderManager* shaderManager = ShaderManager::GetInstance();
	Shader*        cloudsShader  = shaderManager->GetCloudsShader();

	cloudsShader->Use();

	cloudsShader->SetTexture("SceneTexture", sceneTexture, 0);
	cloudsShader->SetTexture("DepthTexture", depthTexture, 1);

	DebugHelper::GetInstance()->FullScreenQuadDrawCall();
}
