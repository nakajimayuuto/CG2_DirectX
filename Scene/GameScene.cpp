#include "GameScene.h"
#include "../Satlib.h"
#include "../ForwardEnemy.h"

GameScene::~GameScene(){
	delete player_;
	delete enemy_;
}

void GameScene::Initialize() {
	TextureManager::GetInstance()->RegisterTexture("uvChecker","Resource/uvChecker.png");
	TextureManager::GetInstance()->RegisterTexture("player_texture","Resource/kari_texture/donut.png");
	TextureManager::GetInstance()->RegisterTexture("bullet_texture","Resource/kari_texture/bullet.png");
	TextureManager::GetInstance()->RegisterTexture("enemy_texture","Resource/kari_texture/kari_musikera.png");

	player_ = new Player();
	player_->Initialize();

	enemy_ = new ForwardEnemy();
	enemy_->Initialize({0.0f,0.0f,50.0f});

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

	if (enemy_) {
		enemy_->Update();
	}

	Camera::GetInstance()->Update();
}

void GameScene::Draw() {
	player_->Draw();

	if (enemy_) {
		enemy_->Draw();
	}
}

void GameScene::RegisterGlobalVariables(){
	Player::RegisterGlobalVariables();
	PlayerBullet::RegisterGlobalVariables();
}										 

void GameScene::ApplyGlobalVariables(){
	Player::ApplyGlobalVariables();
	PlayerBullet::ApplyGlobalVariables();
}
