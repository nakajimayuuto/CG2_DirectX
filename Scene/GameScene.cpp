#include "GameScene.h"
#include "../Satlib.h"

void GameScene::Initialize() {
	ModelManager::GetInstance()->RegisterObj("player","Resource/player","player.obj");
	testModel_.Initialize(ModelManager::GetInstance()->GetModelInfo("player"));
	testTransform_.Initialize();
	testTransform_.rotate.y = Radian(-210.0f);

	Camera::GetInstance()->SetPosition({0.0f,0.0f,-10.0f});
}

void GameScene::Update() {
	Camera::GetInstance()->Update();
}

void GameScene::Draw() {
	testModel_.Draw(testTransform_);
}