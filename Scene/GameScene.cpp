#include "GameScene.h"
#include "../Satlib.h"

GameScene::~GameScene(){
}

void GameScene::Initialize() {
	TextureManager::GetInstance()->RegisterTexture("uvChecker","Resource/uvChecker.png");
	ModelManager::GetInstance()->RegisterObj("skydome","Resource/skydome","skydome.obj");
	ModelManager::GetInstance()->RegisterObj("ground","Resource/Ground","ground.obj");
	ModelManager::GetInstance()->RegisterObj("player","Resource/player_hovering_mode","player.obj");
	ModelManager::GetInstance()->RegisterObj("player_right_arm","Resource/player_hovering_mode/right_arm","right_arm.obj");
	ModelManager::GetInstance()->RegisterObj("player_left_arm","Resource/player_hovering_mode/left_arm","left_arm.obj");
	ModelManager::GetInstance()->RegisterObj("player_head","Resource/player_hovering_mode/head","head.obj");
	ModelManager::GetInstance()->RegisterObj("hammer_of_justice","Resource/Hammer","hammer_of_justice_uv.obj");
	ModelManager::GetInstance()->RegisterObj("enemy","Resource/enemy","enemy.obj");

	Camera::GetInstance()->SetPosition({0.0f,2.0f,-30.0f});

	player_ = std::make_unique<Player>();
	player_->Initialize();

	skydome_ = std::make_unique<Skydome>();
	skydome_->Initialize();

	ground_ = std::make_unique<Ground>();
	ground_->Initialize();

	enemy_ = std::make_unique<Enemy>();
	enemy_->Initialize();

	followCamera_ = std::make_unique<FollowCamera>();
	followCamera_->Initialize();
	followCamera_->SetTarget(player_->GetTransform());

	Player::RegisterGlobalVariables();
}

void GameScene::Update() {
	Player::ApplyGlobalVariables();

	skydome_->Update();

	if (InputManager::GetInstance()->TriggerKey(DIK_R)) {
		SceneManager::GetInstance()->ReloadScene();
	}

	if (InputManager::GetInstance()->TriggerKey(DIK_F3)) {
		Camera::GetInstance()->ChangeCameraMode();
	}

	enemy_->Update();

	player_->Update();

	followCamera_->Update();

	Camera::GetInstance()->Update();
}

void GameScene::Draw() {
	skydome_->Draw();
	ground_->Draw();
	player_->Draw();
	enemy_->Draw();
}
