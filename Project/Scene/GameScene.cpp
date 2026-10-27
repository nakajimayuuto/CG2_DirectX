#include "GameScene.h"
#include "../Satlib.h"
#include "../ProjectileManager.h"
#include "../GamePhase.h"
#include "../EnemyManager.h"

GameScene::~GameScene() {
}

void GameScene::Initialize() {
	tutorialTimer_ = 0.0f;

	SoundManager::GetInstance()->ResetBGM();

	DeltaTime::GetInstance()->SetGameTimeSpeed(1.0f);
	LightManager::GetInstance()->GetDirectionalLightData()->intensity = 0.75f;
	LightManager::GetInstance()->GetDirectionalLightData()->direction = { 0.0f,-1.0f,0.0f };
	LightManager::GetInstance()->GetDirectionalLightData()->color = { 1.0f,1.0f,1.0f,1.0f };
	ParticleManager::GetInstance()->ClearParticles();

	LightManager::GetInstance()->ClearLight();
	Camera::GetInstance()->SetPosition({ 0.0f,2.0f,-30.0f });
	GameCamera::GetInstance()->Initialize();
	DifficultyManager::GetInstance()->SetCurrentDifficulty(kDifficultyNormal);
	player_ = std::make_unique<Player>();
	player_->Initialize();

#ifdef _DEBUG
	GamePhase::GetInstance()->SetCurrentPhase(GamePhase::kBeforeEnter);
#endif // _DEBUG


	//player_->SetStartPosition({ 0.0f,1.0f,-375.0f });
	//if (gGamePhase == GamePhase::kGameStartAnim || gGamePhase == GamePhase::kBossPhase1) {
	//	SoundManager::GetInstance()->SoundPlay("mus_phase1_intro", 1.0f, 0.25f, kBGM, false, "mus_phase1_intro");
	//} else if (gGamePhase == GamePhase::kBossPhaseChangeAnim || gGamePhase == GamePhase::kBossPhase2) {
	//	SoundManager::GetInstance()->SoundPlay("mus_phase2_intro", 1.0f, 0.25f, kBGM, false, "mus_phase2_intro");
	//}

	switch (GamePhase::GetInstance()->GetCurrentPhase()) {
	case GamePhase::kTutorial:

		break;
	case GamePhase::kBeforeEnter:
	case GamePhase::kPhase1:
	case GamePhase::kPhase2:
		player_->SetPosition({ 98.0f,20.0f,0.0f });
		break;
	}

	boss_ = std::make_unique<Boss>();
	boss_->Initialize();
	boss_->SetTargetTransform(player_->GetTransform());
	boss_->SetTargetIsAttact(player_->GetIsAttack());
	//boss_->SetPosition({126.0f,22.0f,0.0f});

	EnemyManager::GetInstance()->Initialize();

	skydome_ = std::make_unique<Skydome>();
	skydome_->Initialize();

	ground_ = std::make_unique<Ground>();
	ground_->Initialize();

	tower_ = std::make_unique<Model>();
	tower_->Initialize("tower");
	tower_->ChangeTexture(TextureManager::GetInstance()->GetTextureInfo("tower_dark"));

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

	isClear_ = false;
	isGameOver_ = false;

	MapChipManager::GetInstance()->Initialize();

	isTutorial_ = true;
	DeltaTime::GetInstance()->Reset();

	CreateUI(20, 6, "block_broken");
	CreateUI(41, 6, "shot_jump");
	CreateUI(48, 11, "shot_jump2");

	std::pair<std::unique_ptr<Model>, Transform> newWall;
	newWall.first = std::make_unique<Model>();
	newWall.first->Initialize("effect_plane");
	TextureInfo textureInfo = TextureManager::GetInstance()->GetTextureInfo("wall_block");
	newWall.first->ChangeTexture(textureInfo);
	newWall.second.Initialize();
	newWall.second.translate = MapChipManager::GetInstance()->GetMapChipField()->GetMapChipPositionByIndex(53, 17);
	newWall.second.translate.x += 0.5f;
	newWall.second.translate.z = 0.5f;
	newWall.first->SetUvTransform(Transform::GetInitialValue({ 108.0f, 32.0f, 1.0f }, { 0.0f,0.0f,0.0f }, { 0.0f,0.0f,0.0f }));
	newWall.second.scale = { 54.0f,16.0f,1.0f };
	uiDatas_.push_back(std::move(newWall));

	newWall.first = std::make_unique<Model>();
	newWall.first->Initialize("effect_plane");
	textureInfo = TextureManager::GetInstance()->GetTextureInfo("wall_block");
	newWall.first->ChangeTexture(textureInfo);
	newWall.second.Initialize();
	newWall.second.translate = MapChipManager::GetInstance()->GetMapChipField()->GetMapChipPositionByIndex(128, 17);
	newWall.second.translate.x += 0.5f;
	newWall.second.translate.z = 0.5f;
	newWall.first->SetUvTransform(Transform::GetInitialValue({ 4.0f, 32.0f, 1.0f }, { 0.0f,0.0f,0.0f }, { 0.0f,0.0f,0.0f }));
	newWall.second.scale = { 2.0f,16.0f,1.0f };
	uiDatas_.push_back(std::move(newWall));
}

void GameScene::CreateUI(uint32_t xIndex, uint32_t yIndex, std::string name) {
	std::pair<std::unique_ptr<Model>, Transform> newUI;
	newUI.first = std::make_unique<Model>();
	newUI.first->Initialize("effect_plane");
	TextureInfo textureInfo = TextureManager::GetInstance()->GetTextureInfo(name);
	newUI.first->ChangeTexture(textureInfo);
	newUI.second.Initialize();
	newUI.second.translate = MapChipManager::GetInstance()->GetMapChipField()->GetMapChipPositionByIndex(xIndex, yIndex);
	newUI.second.scale = { textureInfo.width / 64.0f,textureInfo.height / 64.0f,1.0f };
	uiDatas_.push_back(std::move(newUI));
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

	if (InputManager::GetInstance()->TriggerKey(DIK_F6)) {
		isClear_ = true;
	}

	if (InputManager::GetInstance()->TriggerKey(DIK_F2)) {
		MapChipManager::GetInstance()->ChangeSolidMode();
	}

	if (InputManager::GetInstance()->PressKey(DIK_RETURN)) {
		if (InputManager::GetInstance()->TriggerKey(DIK_1)) {
			ProjectileManager::GetInstance()->CreateLightning(*player_->GetTransform(), CollisionAttributeName::kCollisionEnemyAttack, 10.0f, 3.0f);
		}
		if (InputManager::GetInstance()->TriggerKey(DIK_2)) {
			ProjectileManager::GetInstance()->CreateMeteor(*player_->GetTransform(), CollisionAttributeName::kCollisionEnemyAttack, 10.0f, 3.0f);
		}
		if (InputManager::GetInstance()->TriggerKey(DIK_3)) {
			ProjectileManager::GetInstance()->CreateEnemyBlock(*boss_->GetTransform(), *player_->GetTransform(), CollisionAttributeName::kCollisionEnemyAttack, 10.0f, 1.0f);
		}
	}
#endif // _DEBUG


	if (gameOverMenu_->GetIsActive()) {
		if (gameOverMenu_->GetCanGameUpdate()) {
			gameOverMenu_->Update();
			worldFrameEmitter_->Update();
			worldBigFrameEmitter_->Update();
			fade_->Update();

			Camera::GetInstance()->Update();
			skydome_->Update();
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
	boss_->Update();
	EnemyManager::GetInstance()->Update();

	if (isTutorial_) {
		if (player_->GetWorldPosition().x > kTutorialFinishPosX + 4.0f) {
			SoundManager::GetInstance()->SoundPlay("mus_phase1_intro", 1.0f, 0.25f, kBGM, false, "mus_phase1_intro");
			isTutorial_ = false;
			boss_->FightStart();
			GameCamera::GetInstance()->StartAriaChange(1, 0.5f);
			MapChipManager::GetInstance()->CreateBlock(MapChipManager::GetInstance()->GetMapChipField()->GetMapChipPositionByIndex(static_cast<uint32_t>(kTutorialFinishPosX), 10), 2);
			MapChipManager::GetInstance()->CreateBlock(MapChipManager::GetInstance()->GetMapChipField()->GetMapChipPositionByIndex(static_cast<uint32_t>(kTutorialFinishPosX), 11), 2);
			MapChipManager::GetInstance()->CreateBlock(MapChipManager::GetInstance()->GetMapChipField()->GetMapChipPositionByIndex(static_cast<uint32_t>(kTutorialFinishPosX), 12), 2);
		}
	} else {
		if (!gameOverMenu_->GetIsActive()) {
			if (!boss_->GetIsDeath()) {
				if (SoundManager::GetInstance()->IsFinishedSound("mus_phase1_intro")) {
					SoundManager::GetInstance()->SoundPlay("mus_phase1", 1.0f, 0.25f, kBGM, true, "mus_phase1");
				}
			}
		}
	}


	if (gameOverMenu_->GetIsActive()) {
		if (gameOverMenu_->GetCanGameUpdate()) {
			gameOverMenu_->Update();
			worldFrameEmitter_->Update();
			worldBigFrameEmitter_->Update();
			fade_->Update();

			Camera::GetInstance()->Update();
			skydome_->Update();
			return;
		} else {
			gameOverMenu_->Update();
		}
	} else {
		if (boss_->GetIsDeath()) {
			if (gameclearTimer_ >= gameclearFadeTimerMax_) {
				if (!isClear_) {
					gameclearTimer_ = gameclearFadeTimerMax_;

					isClear_ = true;
					fade_->Start(Fade::Status::FadeOut, 1.0f);
				}
			} else if (gameclearTimer_ >= gameclearTimerMax_) {
				gameclearTimer_ += DeltaTime::GetInstance()->GetDeltaTime();
			} else {
				SoundManager::GetInstance()->SoundStop("mus_phase1");
				gameclearTimer_ += DeltaTime::GetInstance()->GetDeltaTime();
			}
		}
	}

	if (player_->GetIsDeath()) {
		if ((!gameOverMenu_->GetIsActive())) {
			gameOverMenu_->ShowMenu();

			SoundManager::GetInstance()->SoundPause("mus_phase1_intro");
			SoundManager::GetInstance()->SoundPause("mus_phase1");
			//SoundManager::GetInstance()->SoundPause("mus_phase2_intro");
			//SoundManager::GetInstance()->SoundPause("mus_phase2");
		}
	}


	ProjectileManager::GetInstance()->Update();

	GameCamera::GetInstance()->Update();


	worldFrameEmitter_->Update();
	worldBigFrameEmitter_->Update();

	AnimSkipUpdate();
	fade_->Update();
	TutorialUpdate();

	if (boss_->GetIsDeath()) {
		if (gameclearTimer_ >= gameclearFadeTimerMax_) {
			if (isClear_) {
				if (fade_->isFinished()) {
					SceneManager::GetInstance()->ChengeScene(SceneName::kTitleScene);
				}
			}
		}

	}

	Camera::GetInstance()->Update();
	CheckAllCollisions();

	skydome_->Update();
	MapChipManager::GetInstance()->Update();
}

void GameScene::Draw() {
	//ground_->Draw();
	skydome_->Draw();
	Renderer* renderer = Renderer::GetInstance();
	//renderer->DrawSprite(Transform::GetInitialValue( { 1280.0f,720.0f,0.0f },  { 0.0f,0.0f,0.0f }, { 0.0f,0.0f,100.0f }), "white_template", { 0.6f,0.6f,1.0f,1.0f });

	tower_->Draw(Transform::GetInitialValue({ 8.0f,8.0f,8.0f }, { 0.0f,0.0f,0.0f }, { 117.0f,5.0f,30.0f }), false);

	//renderer->DrawModel(Transform::GetInitialValue({ 1.0f,1.0f,1.0f }, { 0.0f,0.0f,0.0f }, { 0.0f,0.0f,0.0f }), "wall", { 1.0f,1.0f,1.0f,1.0f }, false);
	//renderer->DrawBox(Transform::GetInitialValue({ 10.0f,10.0f,1.0f }, { 0.0f,0.0f,0.0f }, { 0.0f,5.0f,-75.0f }), "door", { 1.0f,1.0f,1.0f,1.0f });
	MapChipManager::GetInstance()->Draw();


	ProjectileManager::GetInstance()->Draw();


	player_->Draw();
	boss_->Draw();
	EnemyManager::GetInstance()->Draw();

	TutorialDraw();


	//if (gGamePhase != GamePhase::kTutorial && gGamePhase != GamePhase::kBossLastJarona && gGamePhase != GamePhase::kGameClearStage && gGamePhase != GamePhase::kBossLastJaronaAnim) {

	ParticleManager::GetInstance()->Draw();

	renderer->DrawSprite(Transform::GetInitialValue({ 1.0f,1.0f ,1.0f }, { 0.0f,0.0f,0.0f }, { 500.0f,200.0f,0.0f }), "play_guid", { 1.0f,1.0f,1.0f,1.0f });


	for (auto& ui : uiDatas_) {
		ui.first->Draw(ui.second, false);
	}


	for (auto& wall : wallDatas_) {
		wall.first->Draw(wall.second, false);
	}

	renderer->DrawSprite(Transform::GetInitialValue({ 1.0f,1.0f ,1.0f }, { 0.0f,0.0f,0.0f }, { 265.0f,300.0f,0.0f }), "pause_to_hamburger", { 1.0f,1.0f,1.0f,1.0f });


	if (player_->GetIsDeath()) {
		if (!DeltaTime::GetInstance()->GetIsHitStop()) {
			renderer->DrawSprite(Transform::GetInitialValue(Easing({ 100.0f,100.0f,100.0f }, { 1.0f,1.0f,1.0f }, gameoverTimer_, gameoverTimerMax_, EaseType::kEaseOut), { 0.0f,0.0f,0.0f }, { 0.0f,0.0f,0.0f }), "gameover", { 1.0f,1.0f,1.0f,Easing(0.0f,1.0f, gameoverTimer_, gameoverTimerMax_, EaseType::kEaseOut) });
		}
	}

	if (boss_->GetIsDeath()) {
		renderer->DrawSprite(Transform::GetInitialValue(Easing({ 100.0f,100.0f,100.0f }, { 1.0f,1.0f,1.0f }, gameclearTimer_, gameclearTimerMax_, EaseType::kEaseOut), { 0.0f,0.0f,0.0f }, { 0.0f,0.0f,0.0f }), "gameclear", { 1.0f,1.0f,1.0f,Easing(0.0f,1.0f, gameclearTimer_, gameclearTimerMax_, EaseType::kEaseOut) });
	}

	fade_->Draw();
	pauseMenu_->Draw();
	gameOverMenu_->Draw();
}

void GameScene::CheckAllCollisions() {
	CollisionManager::GetInstance()->CheckAllCollision();
}

void GameScene::AnimSkipUpdate() {
	return;
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
