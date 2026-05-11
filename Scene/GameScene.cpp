#include "GameScene.h"
#include "../Satlib.h"

GameScene::~GameScene(){
	delete player_;
}

void GameScene::Initialize() {
	TextureManager::GetInstance()->RegisterTexture("uvChecker","Resource/uvChecker.png");
	TextureManager::GetInstance()->RegisterTexture("player_texture","Resource/kari_texture/donut.png");
	TextureManager::GetInstance()->RegisterTexture("bullet_texture","Resource/kari_texture/bullet.png");

	player_ = new Player();
	player_->Initialize();

	RegisterGlobalVariables();
}

void GameScene::Update() {
#ifdef _DEBUG
	if (InputManager::GetInstance()->TriggerKey(DIK_F3)) {
		Camera::GetInstance()->ChangeCameraMode();
	}

	if (InputManager::GetInstance()->TriggerKey(DIK_R)) {
		SceneManager::GetInstance()->ReloadScene();
	}
#endif // _DEBUG
	ApplyGlobalVariables();

	player_->Update();

	Camera::GetInstance()->Update();
}

void GameScene::Draw() {
	player_->Draw();
}

void GameScene::RegisterGlobalVariables(){
	Player::RegisterGlobalVariables();
	PlayerBullet::RegisterGlobalVariables();
}										 

void GameScene::ApplyGlobalVariables(){
	Player::ApplyGlobalVariables();
	PlayerBullet::ApplyGlobalVariables();
}
