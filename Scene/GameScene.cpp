#include "GameScene.h"
#include "../Satlib.h"
#include "../ProjectileManager.h"

GameScene::~GameScene() {
}

void GameScene::Initialize() {
	TutorialManager::GetInstance()->Initialize();


	LightManager::GetInstance()->ClearLight();
	Camera::GetInstance()->SetPosition({ 0.0f,2.0f,-30.0f });
	GameCamera::GetInstance()->Initialize();
	player_ = std::make_unique<Player>();
	player_->Initialize();
	player_->SetStartPosition({ 0.0f,1.0f,-375.0f });

	skydome_ = std::make_unique<Skydome>();
	skydome_->Initialize();

	ground_ = std::make_unique<Ground>();
	ground_->Initialize();


	boss_ = std::make_unique<Boss>();
	boss_->Initialize();
	boss_->SetTargetTransform(player_->GetTransform());

	ProjectileManager::GetInstance()->Initialize();

	worldFrameEmitter_ = std::make_unique<Emitter>();
	worldFrameEmitter_->SetParticle(ParticleManager::GetInstance()->GetParticles("fire"));
	// 第一形態.
	//worldFrameEmitter_->Initialize(Transform::GetInitialValue({ 150.0f,10.0f,150.0f }, { 0.0f,0.0f,0.0f }, {0.0f,5.0f,0.0f}), 10, 0.1f);
	// 第二形態.
	worldFrameEmitter_->Initialize(Transform::GetInitialValue({ 150.0f,10.0f,150.0f }, { 0.0f,0.0f,0.0f }, { 0.0f,5.0f,0.0f }), 30, 0.1f);

	worldBigFrameEmitter_ = std::make_unique<Emitter>();
	worldBigFrameEmitter_->SetParticle(ParticleManager::GetInstance()->GetParticles("big_fire"));
	worldBigFrameEmitter_->Initialize(Transform::GetInitialValue({ 150.0f,10.0f,150.0f }, { 0.0f,0.0f,0.0f }, { 0.0f,5.0f,0.0f }), 5, 0.2f);

	tutorialAttackWall_ = std::make_unique<TutorialObject>();
	tutorialAttackWall_->Initialize(1.0f);
	tutorialAttackWall_->transform_.translate = { 0.0f,5.0f,-340.0f };

	tutorialExitWall_ = std::make_unique<TutorialObject>();
	tutorialExitWall_->Initialize(2.0f);
	tutorialExitWall_->transform_.translate = { 0.0f,5.0f,-75.0f };

	Player::RegisterGlobalVariables();

	fade_->Initialize();
	fade_->Start(Fade::Status::FadeOut, 1.0f);
	//gGamePhase = GamePhase::kBossLastJarona;
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

	if (InputManager::GetInstance()->TriggerKey(DIK_F4)) {
		Renderer::GetInstance()->ChangeUseDebugLine();
	}

	player_->Update();

	boss_->SetTargetIsAttact(player_->GetIsAttack());

	boss_->Update();

	ProjectileManager::GetInstance()->Update();

	GameCamera::GetInstance()->Update();

	Camera::GetInstance()->Update();

	worldFrameEmitter_->Update();
	worldBigFrameEmitter_->Update();

	fade_->Update();
	TutorialUpdate();

	CheckAllCollisions();
}

void GameScene::Draw() {
	ground_->Draw();

	if (gGamePhase == GamePhase::kTutorial) {
		Renderer::GetInstance()->DrawModel(Transform::GetInitialValue({ 1.0f,1.0f,1.0f }, { 0.0f,0.0f,0.0f }, { 0.0f,5.0f,-275.0f }), "tutorial_wall", { 1.0f,1.0f,1.0f,1.0f }, false);

		Renderer::GetInstance()->DrawModel(Transform::GetInitialValue({ 1.0f,0.5f,1.0f }, { 0.0f,0.0f,0.0f }, { 0.0f,0.0f,0.0f }), "wall", { 1.0f,1.0f,1.0f,0.3f }, false);
	} else if (gGamePhase == GamePhase::kBossLastJarona || gGamePhase == GamePhase::kGameClearStage) {
		Renderer::GetInstance()->DrawModel(Transform::GetInitialValue({ 1.0f,1.0f,1.0f }, { 0.0f,0.0f,0.0f }, { 0.0f,5.0f,-275.0f }), "tutorial_wall", { 1.0f,1.0f,1.0f,1.0f }, false);
	} else if (gGamePhase == GamePhase::kBossLastJaronaAnim) {
		Renderer::GetInstance()->DrawModel(Transform::GetInitialValue({ 1.0f,1.0f,1.0f }, { 0.0f,0.0f,0.0f }, { 0.0f,5.0f,-275.0f }), "tutorial_wall", { 1.0f,1.0f,1.0f,1.0f }, false);

		Renderer::GetInstance()->DrawModel(Transform::GetInitialValue({ 1.0f,0.5f,1.0f }, { 0.0f,0.0f,0.0f }, { 0.0f,0.0f,0.0f }), "wall", { 1.0f,1.0f,1.0f,1.0f }, false);
	} else {
		Renderer::GetInstance()->DrawModel(Transform::GetInitialValue({ 1.0f,0.5f,1.0f }, { 0.0f,0.0f,0.0f }, { 0.0f,0.0f,0.0f }), "wall", { 1.0f,1.0f,1.0f,1.0f }, false);
	}

	ProjectileManager::GetInstance()->Draw();


	player_->Draw();
	boss_->Draw();

	TutorialDraw();

	fade_->Draw();
}

void GameScene::CheckAllCollisions() {
	CollisionManager::GetInstance()->CheckAllCollision();
}

void GameScene::TutorialUpdate() {
	if (gGamePhase != GamePhase::kTutorial) {
		return;
	}

	tutorialAttackWall_->CollisionUpdate();
	tutorialExitWall_->CollisionUpdate();
	if (TutorialManager::GetInstance()->GetCurrentFlagName() == TutorialManager::TutorialFlagName::kAttackTest) {
		if (!tutorialAttackWall_->isActive_) {
			TutorialManager::GetInstance()->NextTutorial();
		}

	} else if (TutorialManager::GetInstance()->GetCurrentFlagName() == TutorialManager::TutorialFlagName::kFinaleTest) {
		if (!tutorialExitWall_->isActive_) {
			if (fade_->isFinished()) {
				fade_->Start(Fade::Status::FadeIn, 1.0f);
			}
		}

	}

	if (tutorialAttackWall_->isActive_) {
		player_->SetTutorialClamp(tutorialAttackWall_->transform_.translate.z);
	} else if (tutorialExitWall_->isActive_) {
		player_->SetTutorialClamp(tutorialExitWall_->transform_.translate.z);
	} else {
		player_->SetTutorialClamp(80.0f);
	}
}

void GameScene::TutorialDraw() {
	if (gGamePhase != GamePhase::kTutorial) {
		return;
	}

	tutorialAttackWall_->Draw();
	tutorialExitWall_->Draw();
}
