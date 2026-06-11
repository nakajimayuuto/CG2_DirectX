#include "GameScene.h"
#include "../Satlib.h"

GameScene::~GameScene(){
}

void GameScene::Initialize() {
	TextureManager::GetInstance()->RegisterTexture("uvChecker","Resource/uvChecker.png");
	ModelManager::GetInstance()->RegisterObj("skydome","Resource/skydome","skydome.obj");

	

	player_ = std::make_unique<Player>();
	player_->Initialize();

	skydome_ = std::make_unique<Skydome>();
	skydome_->Initialize();
}

void GameScene::Update() {
	if (InputManager::GetInstance()->TriggerKey(DIK_R)) {
		SceneManager::GetInstance()->ReloadScene();
	}

	if (InputManager::GetInstance()->TriggerKey(DIK_F3)) {
		Camera::GetInstance()->ChangeCameraMode();
	}

	player_->Update();

	Camera::GetInstance()->Update();
}

void GameScene::Draw() {
	skydome_->Draw();
	player_->Draw();
}
