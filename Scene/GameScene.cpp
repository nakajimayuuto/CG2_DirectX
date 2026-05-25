#include "GameScene.h"

void GameScene::Initialize() {
	ModelManager::GetInstance()->RegisterObj("player","Resource/player","player.obj");
	transform_.Initialize();
	model_.Initialize(ModelManager::GetInstance()->GetModelInfo("player"));
}

void GameScene::Update() {
	if (InputManager::GetInstance()->TriggerKey(DIK_F11)) {
		if (Environment::GetInstance()->GetWindowMode() == kFullscreen) {
			Environment::GetInstance()->SetWindowMode(kWindowed);
		} else {
			Environment::GetInstance()->SetWindowMode(kFullscreen);
		}
	}
}

void GameScene::Draw() {
	model_.Draw(transform_);
}