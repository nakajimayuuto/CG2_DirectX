#include "GameScene.h"
#include "../Satlib.h"
#include "../ProjectileManager.h"

GameScene::~GameScene() {
}

void GameScene::Initialize() {

	Camera::GetInstance()->SetPosition({ 0.0f,2.0f,-30.0f });
	GameCamera::GetInstance()->Initialize();
	player_ = std::make_unique<Player>();
	player_->Initialize();

	skydome_ = std::make_unique<Skydome>();
	skydome_->Initialize();

	ground_ = std::make_unique<Ground>();
	ground_->Initialize();


	boss_ = std::make_unique<Boss>();
	boss_->Initialize();
	boss_->SetTargetTransform(player_->GetTransform());

	ProjectileManager::GetInstance()->Initialize();

	Player::RegisterGlobalVariables();
}

void GameScene::Update() {
	DifficultyManager::GetInstance()->Update();
	CollisionManager::GetInstance()->ClearColliderList();
	Player::ApplyGlobalVariables();

	skydome_->Update();

	if (InputManager::GetInstance()->TriggerKey(DIK_R)) {
		SceneManager::GetInstance()->ReloadScene();
	}

	if (InputManager::GetInstance()->TriggerKey(DIK_F3)) {
		Camera::GetInstance()->ChangeCameraMode();
	}

	player_->Update();
	boss_->SetTargetIsAttact(player_->GetIsAttack());

	boss_->Update();

	ProjectileManager::GetInstance()->Update();

	GameCamera::GetInstance()->Update();

	Camera::GetInstance()->Update();

	CheckAllCollisions();
}

void GameScene::Draw() {
	skydome_->Draw();
	ground_->Draw();

	Renderer::GetInstance()->SetBlendMode(BlendMode::kNormalCullNone);
	Renderer::GetInstance()->DrawBox(Transform::GetInitialValue({ 150.0f,50.0f,150.0f }, { 0.0f,0.0f,0.0f }, { 0.0f,0.0f,0.0f }), "uvChecker", { 1.0f,1.0f,1.0f,1.0f });
	Renderer::GetInstance()->DrawBox(Transform::GetInitialValue({ 150.0f,50.0f,150.0f }, { 0.0f,Radian(45.0f),0.0f }, { 0.0f,0.0f,0.0f }), "uvChecker", { 1.0f,1.0f,1.0f,1.0f });
	Renderer::GetInstance()->SetBlendMode(BlendMode::kNormal);

	ProjectileManager::GetInstance()->Draw();

	player_->Draw();
	boss_->Draw();
}

void GameScene::CheckAllCollisions() {
	CollisionManager::GetInstance()->CheckAllCollision();
}
