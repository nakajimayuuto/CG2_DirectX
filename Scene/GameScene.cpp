#include "GameScene.h"

void GameScene::Initialize() {
	//transform_.Initialize();
	//model_.Initialize();
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
}