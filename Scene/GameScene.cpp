#include "GameScene.h"
#include "../Engine/Renderer/Camera.h"
#include "../Engine/SystemFile/ImGui.h"
#include "../Engine/Math/Math.h"
#include "../Engine/Renderer/DirectionalLight.h"
#include "../Managers/SoundManager.h"
#include "../Managers/InputManager.h"
#include "../Environment.h"

void GameScene::Initialize() {
	ModelManager::GetInstance()->RegisterObj("test", "Resource/Evaluation", "suzanne.obj");
	testModel.Initialize(ModelManager::GetInstance()->GetModelInfo("test"));
	ModelManager::GetInstance()->RegisterObj("testPlane", "Resource", "plane.obj");
	testMultiModel.Initialize(ModelManager::GetInstance()->GetModelInfo("testPlane"));
}

void GameScene::Update() {
	if (InputManager::GetInstance()->TriggerKey(DIK_ESCAPE)) {
		Environment::GetInstance()->GameFinished();
	}

	Camera::GetInstance()->Update();

	Vector4 imColor = testModel.GetColor();

	ImGui::Begin("Color");
	ImGui::ColorEdit4("modelColor",reinterpret_cast<float*>(&imColor.x));
	ImGui::End();

	transformModel.rotate = Degree(transformModel.rotate);

	testModel.SetColor(imColor);
	ImGui::Begin("Plane");
	ImGui::SliderFloat3("Rotate", reinterpret_cast<float*>(&transformModel.rotate.x),-360.0f,360.0f);
	ImGui::End();

	transformModel.rotate = Radian(transformModel.rotate);
}

void GameScene::Draw() {
	testModel.Draw({ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{1.0f,0.0f,0.0f} });
	//GameSystem::GetInstance()->CreatePipeline(D3D12_CULL_MODE_NONE);
	//GameSystem::GetInstance()->AdaptPipeline();
	testMultiModel.Draw(transformModel);
}