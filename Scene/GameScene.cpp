#include "GameScene.h"
#include "../Satlib.h"
#include "../ProjectileManager.h"

GameScene::~GameScene() {
}

void GameScene::Initialize() {
	tutorialTimer_ = 0.0f;

	SoundManager::GetInstance()->ResetBGM();

	DeltaTime::GetInstance()->SetGameTimeSpeed(1.0f);
	LightManager::GetInstance()->GetDirectionalLightData()->intensity = 0.5f;
	LightManager::GetInstance()->GetDirectionalLightData()->direction = { 0.0f,-1.0f,0.0f };
	LightManager::GetInstance()->GetDirectionalLightData()->color = { 1.0f,1.0f,1.0f,1.0f };
	ParticleManager::GetInstance()->ClearParticles();

	LightManager::GetInstance()->ClearLight();
	Camera::GetInstance()->SetPosition({ 0.0f,2.0f,-30.0f });
	GameCamera::GetInstance()->Initialize();
	player_ = std::make_unique<Player>();
	player_->Initialize();
	//player_->SetStartPosition({ 0.0f,1.0f,-375.0f });
	//if (gGamePhase == GamePhase::kGameStartAnim || gGamePhase == GamePhase::kBossPhase1) {
	//	SoundManager::GetInstance()->SoundPlay("mus_phase1_intro", 1.0f, 0.25f, kBGM, false, "mus_phase1_intro");
	//} else if (gGamePhase == GamePhase::kBossPhaseChangeAnim || gGamePhase == GamePhase::kBossPhase2) {
	//	SoundManager::GetInstance()->SoundPlay("mus_phase2_intro", 1.0f, 0.25f, kBGM, false, "mus_phase2_intro");
	//}

	skydome_ = std::make_unique<Skydome>();
	skydome_->Initialize();

	ground_ = std::make_unique<Ground>();
	ground_->Initialize();

	ProjectileManager::GetInstance()->Initialize();

	worldFrameEmitter_ = std::make_unique<Emitter>();
	worldFrameEmitter_->SetParticle(ParticleManager::GetInstance()->GetParticles("fire"));
	// 第一形態.
	//worldFrameEmitter_->Initialize(Transform::GetInitialValue({ 150.0f,10.0f,150.0f }, { 0.0f,0.0f,0.0f }, {0.0f,5.0f,0.0f}), 10, 0.1f);
	// 第二形態.
	worldFrameEmitter_->Initialize(Transform::GetInitialValue({ 150.0f,20.0f,150.0f }, { 0.0f,0.0f,0.0f }, { 0.0f,10.0f,0.0f }), 30, 0.1f);

	worldBigFrameEmitter_ = std::make_unique<Emitter>();
	worldBigFrameEmitter_->SetParticle(ParticleManager::GetInstance()->GetParticles("big_fire"));
	worldBigFrameEmitter_->Initialize(Transform::GetInitialValue({ 150.0f,20.0f,150.0f }, { 0.0f,0.0f,0.0f }, { 0.0f,10.0f,0.0f }), 5, 0.2f);

	Player::RegisterGlobalVariables();


	fade_ = std::make_unique<Fade>();
	fade_->Initialize();
	fade_->SetColor({ 0.0f,0.0f,0.0f });
	fade_->Start(Fade::Status::FadeIn, 1.0f);
	useSkipStart_ = false;
	useSkipEnd_ = false;

	pauseMenu_ = std::make_unique<PauseMenu>();
	pauseMenu_->Initialize();

	gameOverMenu_ = std::make_unique<GameOverMenu>();
	gameOverMenu_->Initialize();
	//gGamePhase = GamePhase::kBossLastJarona;

	isBossDeath_ = false;

	MapChipManager::GetInstance()->Initialize();
}

void GameScene::Update() {
	DifficultyManager::GetInstance()->Update();
	CollisionManager::GetInstance()->ClearColliderList();
	Player::ApplyGlobalVariables();

	skydome_->Update();
#ifdef _DEBUG

	if (InputManager::GetInstance()->TriggerKey(DIK_R)) {
		SceneManager::GetInstance()->ReloadScene();
	}

	if (InputManager::GetInstance()->TriggerKey(DIK_F3)) {
		Camera::GetInstance()->ChangeCameraMode();
	}

	if (InputManager::GetInstance()->TriggerKey(DIK_F4)) {
		Renderer::GetInstance()->ChangeUseDebugLine();
	}
#endif // _DEBUG

	if (gameOverMenu_->GetIsActive()) {
		if (gameOverMenu_->GetCanGameUpdate()) {
			gameOverMenu_->Update();
			worldFrameEmitter_->Update();
			worldBigFrameEmitter_->Update();
			fade_->Update();

			Camera::GetInstance()->Update();
			return;
		} else {
			gameOverMenu_->Update();
		}
	}

	if (pauseMenu_->GetIsActive()) {
		pauseMenu_->Update();
		worldFrameEmitter_->Update();
		worldBigFrameEmitter_->Update();
		fade_->Update();

		Camera::GetInstance()->Update();
		return;
	} else {
		if (!useSkipStart_) {
			if (!gameOverMenu_->GetIsActive()) {
				if (InputManager::GetInstance()->TriggerPadButton(INPUT_START) || InputManager::GetInstance()->TriggerKey(DIK_P)) {
					pauseMenu_->ShowMenu();
				}

			}
		}
	}

	player_->Update();


	ProjectileManager::GetInstance()->Update();

	GameCamera::GetInstance()->Update();


	worldFrameEmitter_->Update();
	worldBigFrameEmitter_->Update();

	AnimSkipUpdate();
	fade_->Update();
	TutorialUpdate();

	Camera::GetInstance()->Update();
	CheckAllCollisions();

	MapChipManager::GetInstance()->Update();
}

void GameScene::Draw() {
	ground_->Draw();
	Renderer* renderer = Renderer::GetInstance();


	//renderer->DrawModel(Transform::GetInitialValue({ 1.0f,1.0f,1.0f }, { 0.0f,0.0f,0.0f }, { 0.0f,0.0f,0.0f }), "wall", { 1.0f,1.0f,1.0f,1.0f }, false);
	//renderer->DrawBox(Transform::GetInitialValue({ 10.0f,10.0f,1.0f }, { 0.0f,0.0f,0.0f }, { 0.0f,5.0f,-75.0f }), "door", { 1.0f,1.0f,1.0f,1.0f });
	MapChipManager::GetInstance()->Draw();


	ProjectileManager::GetInstance()->Draw();


	player_->Draw();

	TutorialDraw();


	//if (gGamePhase != GamePhase::kTutorial && gGamePhase != GamePhase::kBossLastJarona && gGamePhase != GamePhase::kGameClearStage && gGamePhase != GamePhase::kBossLastJaronaAnim) {

	ParticleManager::GetInstance()->Draw();

	renderer->DrawSprite(Transform::GetInitialValue({ 1.0f,1.0f ,1.0f }, { 0.0f,0.0f,0.0f }, { 500.0f,200.0f,0.0f }), "play_guid", { 1.0f,1.0f,1.0f,1.0f });




	renderer->DrawSprite(Transform::GetInitialValue({ 1.0f,1.0f ,1.0f }, { 0.0f,0.0f,0.0f }, { 265.0f,300.0f,0.0f }), "pause_to_hamburger", { 1.0f,1.0f,1.0f,1.0f });


	if (player_->GetIsDeath()) {
		if (!DeltaTime::GetInstance()->GetIsHitStop()) {
			renderer->DrawSprite(Transform::GetInitialValue(Easing({ 100.0f,100.0f,100.0f }, { 1.0f,1.0f,1.0f }, gameoverTimer_, gameoverTimerMax_, EaseType::kEaseOut), { 0.0f,0.0f,0.0f }, { 0.0f,0.0f,0.0f }), "gameover", { 1.0f,1.0f,1.0f,Easing(0.0f,1.0f, gameoverTimer_, gameoverTimerMax_, EaseType::kEaseOut) });
		}
	}

	fade_->Draw();
	pauseMenu_->Draw();
	gameOverMenu_->Draw();
}

void GameScene::CheckAllCollisions() {
	CollisionManager::GetInstance()->CheckAllCollision();
}

void GameScene::AnimSkipUpdate() {

	if (!fade_->isFinished()) {
		return;
	}

	if (!useSkipStart_) {
		if (InputManager::GetInstance()->IsGamePadConnect()) {
			if (InputManager::GetInstance()->PressPadButton(INPUT_L1) && InputManager::GetInstance()->PressPadButton(INPUT_L1)) {
				fade_->SetColor({ 0.0f,0.0f,0.0f });
				fade_->Start(Fade::Status::FadeOut, 1.0f);
				useSkipStart_ = true;
			}
		} else {
			if (InputManager::GetInstance()->TriggerKey(DIK_I) || InputManager::GetInstance()->TriggerKey(DIK_Q)) {
				fade_->SetColor({ 0.0f,0.0f,0.0f });
				fade_->Start(Fade::Status::FadeOut, 1.0f);
				useSkipStart_ = true;
			}
		}
	}

	AnimSkipFadeUpdate();
}

void GameScene::AnimSkipFadeUpdate() {
}

void GameScene::TutorialInitialize() {
	for (uint32_t i = 0; i < 7; i++) {

		ProjectileManager::GetInstance()->CreateTutorialObstacle({ (static_cast<float>(i) - 3.0f) * 2.0f,2.0f,-310.0f }, TutorialObstaclesType::kBullet);
	}
	for (uint32_t i = 0; i < 9; i++) {

		ProjectileManager::GetInstance()->CreateTutorialObstacle({ (static_cast<float>(i) - 4.0f),2.0f,-260.0f }, TutorialObstaclesType::kSpike);
	}

	for (uint32_t i = 0; i < 7; i++) {

		ProjectileManager::GetInstance()->CreateTutorialObstacle({ (static_cast<float>(i) - 3.0f) * 2.0f,2.0f,-140.0f }, TutorialObstaclesType::kBullet);
	}
	for (uint32_t i = 0; i < 9; i++) {

		ProjectileManager::GetInstance()->CreateTutorialObstacle({ (static_cast<float>(i) - 4.0f),2.0f,-90.0f }, TutorialObstaclesType::kSpike);
	}
}

void GameScene::TutorialUpdate() {
}

void GameScene::TutorialDraw() {
}
