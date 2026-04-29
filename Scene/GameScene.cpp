#include "GameScene.h"
#include "../Satlib.h"

GameScene::~GameScene(){
	delete player_;
}

void GameScene::Initialize() {
	TextureManager::GetInstance()->RegisterTexture("uvChecker","Resource/uvChecker.png");

	player_ = new Player();
	player_->Initialize();
}

void GameScene::Update() {

	player_->Update();

	Camera::GetInstance()->Update();
}

void GameScene::Draw() {
	player_->Draw();
}