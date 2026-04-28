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

	testModel.SetColor(imColor);
}

void GameScene::Draw() {
	testModel.Draw({ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} });
}