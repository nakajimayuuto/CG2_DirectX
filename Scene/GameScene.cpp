#include "GameScene.h"
#include "../Satlib.h"

void GameScene::Initialize() {
	ModelManager::GetInstance()->RegisterObj("player","Resource/player","player.obj");
	ModelManager::GetInstance()->RegisterObj("block","Resource/block","block.obj");
	ModelManager::GetInstance()->RegisterObj("plane","Resource","plane.obj");
	ModelManager::GetInstance()->RegisterObj("fence","Resource/fence","fence.obj");
	testModel_.Initialize(ModelManager::GetInstance()->GetModelInfo("fence"));
	testTransform_.Initialize();
	testTransform_.translate = {0.0f,0.0f,0.0f};
	testTransform_.rotate.y = Radian(-180.0f);
	testTransform_.rotate.x = Radian(30.0f);

	Camera::GetInstance()->SetPosition({0.0f,0.0f,-10.0f});
}

void GameScene::Update() {

	ImGui::Begin("Color");

	Vector4 color = testModel_.GetColor();

	ImGui::ColorEdit4("testModelPlayer",reinterpret_cast<float*>(&color));

	ImGui::SliderFloat3("PlayerTransform",reinterpret_cast<float*>(&testTransform_.rotate),-6.0f,6.0f);

	testModel_.SetColor(color);
	
	//color = testModel2_.GetColor();
	//
	//ImGui::ColorEdit4("testModelBlock",reinterpret_cast<float*>(&color));
	//
	//ImGui::SliderFloat3("PlayerTransform",reinterpret_cast<float*>(&testTransform2_.rotate),-6.0f,6.0f);
	//
	//testModel2_.SetColor(color);

	ImGui::End();

	Camera::GetInstance()->Update();
}

void GameScene::Draw() {

	testModel_.Draw(testTransform_);

	//testModel2_.Draw(testTransform2_);
}