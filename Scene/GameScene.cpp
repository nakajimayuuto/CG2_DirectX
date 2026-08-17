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

	fade_ = std::make_unique<Fade>();
	fade_->Initialize();
	fade_->SetColor({ 0.0f,0.0f,0.0f });
	fade_->Start(Fade::Status::FadeIn, 1.0f);
	useSkipStart_ = false;
	useSkipEnd_ = false;
	//gGamePhase = GamePhase::kBossLastJarona;
}

void GameScene::Update() {
	DifficultyManager::GetInstance()->Update();
	CollisionManager::GetInstance()->ClearColliderList();
	Player::ApplyGlobalVariables();

	skydome_->Update();
#ifdef _DEBUG
	ImGui::Begin("GamePhase");
	ImGui::Text(magic_enum::enum_name(gGamePhase).data());
	ImGui::End();
#endif // _DEBUG


	if (InputManager::GetInstance()->TriggerKey(DIK_R)) {
		SceneManager::GetInstance()->ReloadScene();
	}

	if (InputManager::GetInstance()->TriggerKey(DIK_F3)) {
		Camera::GetInstance()->ChangeCameraMode();
	}

	if (InputManager::GetInstance()->TriggerKey(DIK_F4)) {
		Renderer::GetInstance()->ChangeUseDebugLine();
	}

	if (InputManager::GetInstance()->TriggerKey(DIK_F2)) {
		gGamePhase = GamePhase::kTutorial;
	}

	player_->Update();

	boss_->SetTargetIsAttact(player_->GetIsAttack());

	boss_->Update();

	ProjectileManager::GetInstance()->Update();

	GameCamera::GetInstance()->Update();


	worldFrameEmitter_->Update();
	worldBigFrameEmitter_->Update();

	AnimSkipUpdate();

	fade_->Update();
	TutorialUpdate();

	Camera::GetInstance()->Update();
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

	ParticleManager::GetInstance()->Draw();

	fade_->Draw();
}

void GameScene::CheckAllCollisions() {
	CollisionManager::GetInstance()->CheckAllCollision();
}

void GameScene::AnimSkipUpdate() {
	if (gGamePhase != GamePhase::kTutorial && 
		gGamePhase != GamePhase::kGameStartAnim &&
		gGamePhase != GamePhase::kBossPhaseChangeAnim &&
		gGamePhase != GamePhase::kBossLastJaronaAnim) {
		return;
	} 

	if (gGamePhase == GamePhase::kTutorial) {
		if (InputManager::GetInstance()->IsGamePadConnect()) {
			if (InputManager::GetInstance()->TriggerPadButton(INPUT_START)) {
				fade_->SetColor({0.0f,0.0f,0.0f});
				fade_->Start(Fade::Status::FadeOut, 1.0f);
				useSkipStart_ = true;
			}
		} else {
			if (InputManager::GetInstance()->TriggerKey(DIK_P)) {
				fade_->SetColor({ 0.0f,0.0f,0.0f });
				fade_->Start(Fade::Status::FadeOut, 1.0f);
				useSkipStart_ = true;
			}
		}

		AnimSkipFadeUpdate();
		return;
	} 

	if (InputManager::GetInstance()->IsGamePadConnect()) {
		if (InputManager::GetInstance()->TriggerPadButton(INPUT_A)) {
			fade_->SetColor({ 0.0f,0.0f,0.0f });
			fade_->Start(Fade::Status::FadeOut, 1.0f);
			useSkipStart_ = true;
		}
	} else {
		if (InputManager::GetInstance()->TriggerKey(DIK_SPACE)) {
			fade_->SetColor({ 0.0f,0.0f,0.0f });
			fade_->Start(Fade::Status::FadeOut, 1.0f);
			useSkipStart_ = true;
		}
	}

	AnimSkipFadeUpdate();
}

void GameScene::AnimSkipFadeUpdate(){
	if (useSkipStart_) {
		if (fade_->isFinished()) {
			gGamePhase = static_cast<GamePhase>(static_cast<uint32_t>(gGamePhase) + 1);
			useSkipStart_ = false;
			useSkipEnd_ = true;

			switch (gGamePhase) {
			case kTutorial:
				break;
			case kGameStartAnim:
				player_->Initialize();
				player_->SetStartPosition({ 0.0f,1.0f,-50.0f });
				boss_->Initialize();
				break;
			case kBossPhase1:
				break;
			case kBossPhaseChangeAnim:
				break;
			case kBossPhase2:
				break;
			case kBossLastJaronaAnim:
				break;
			case kBossLastJarona:
				break;
			case kGameClearStage:
				break;
			default:
				break;
			}

			fade_->Start(Fade::Status::FadeIn, 1.0f);
		}
	} else if (useSkipEnd_) {
		if (fade_->isFinished()) {
			useSkipEnd_ = false;
		}

	}
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
				fade_->SetColor({ 1.0f,1.0f,1.0f });
				fade_->Start(Fade::Status::FadeOut, 1.0f);
				TutorialManager::GetInstance()->NextTutorial();
			}
		}
	} else if (TutorialManager::GetInstance()->GetCurrentFlagName() == TutorialManager::TutorialFlagName::kFinaleAnim) {
		if (fade_->isFinished()) {
			fade_->Start(Fade::Status::FadeIn, 1.0f);
			gGamePhase = GamePhase::kGameStartAnim;
			player_->Initialize();
			player_->SetStartPosition({ 0.0f,1.0f,-50.0f });
			boss_->Initialize();
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
