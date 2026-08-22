#include "GameScene.h"
#include "../Satlib.h"
#include "../ProjectileManager.h"

GameScene::~GameScene() {
}

void GameScene::Initialize() {
	LightManager::GetInstance()->GetDirectionalLightData()->intensity = 0.15f;
	LightManager::GetInstance()->GetDirectionalLightData()->direction = { 0.0f,-1.0f,0.0f };
	LightManager::GetInstance()->GetDirectionalLightData()->color = { 1.0f,1.0f,1.0f,1.0f };
	//LightManager::GetInstance()->GetDirectionalLightData()->intensity = 0.1f;
	//LightManager::GetInstance()->GetDirectionalLightData()->color = { 1.0f,0.5f,0.5f,1.0f };
	TutorialManager::GetInstance()->Initialize();


	LightManager::GetInstance()->ClearLight();
	Camera::GetInstance()->SetPosition({ 0.0f,2.0f,-30.0f });
	GameCamera::GetInstance()->Initialize();
	player_ = std::make_unique<Player>();
	player_->Initialize();
	player_->SetStartPosition({ 0.0f,1.0f,-375.0f });

	if (gGamePhase == GamePhase::kBossLastJaronaAnim) {
		player_->SetStartPosition({ 0.0f,1.0f,-50.0f });
	} else if (gGamePhase == GamePhase::kBossPhaseChangeAnim) {
		player_->SetStartPosition({ 0.0f,1.0f,-50.0f });
	} else if (gGamePhase == GamePhase::kGameStartAnim) {
		player_->SetStartPosition({ 0.0f,1.0f,-50.0f });
	}

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
	tutorialAttackWall_->transform_.translate = { 0.0f,10.0f,-340.0f };

	tutorialExitWall_ = std::make_unique<TutorialObject>();
	tutorialExitWall_->Initialize(2.0f);
	tutorialExitWall_->transform_.translate = { 0.0f,10.0f,-75.0f };

	Player::RegisterGlobalVariables();

	fade_ = std::make_unique<Fade>();
	fade_->Initialize();
	fade_->SetColor({ 0.0f,0.0f,0.0f });
	fade_->Start(Fade::Status::FadeIn, 1.0f);
	useSkipStart_ = false;
	useSkipEnd_ = false;

	pauseMenu_ = std::make_unique<PauseMenu>();
	pauseMenu_->Initialize();
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

	if (pauseMenu_->GetIsActive()) {
		pauseMenu_->Update();
		worldFrameEmitter_->Update();
		worldBigFrameEmitter_->Update();
		fade_->Update();

		Camera::GetInstance()->Update();
		return;
	} else {
		if (InputManager::GetInstance()->TriggerPadButton(INPUT_START) || InputManager::GetInstance()->TriggerKey(DIK_P)) {
			pauseMenu_->ShowMenu();
		}
	}

	if (boss_->GetIsChangePhase()) {
		boss_->SetIsImmune(true);
		player_->SetIsImmune(true);
		boss_->SetIsChangePhase(false);
		fade_->Start(Fade::Status::FadeOut, 1.0f);
	}

	if (gGamePhase == GamePhase::kBossPhase1) {
		if (boss_->GetPhase() == Boss::Phase::kPhase2) {
			if (fade_->isFinished()) {
				gGamePhase = GamePhase::kBossPhaseChangeAnim;
				SceneManager::GetInstance()->ReloadScene();
			}
		}
	}

	player_->Update();

	boss_->SetTargetIsAttact(player_->GetIsAttack());
	if (gGamePhase != GamePhase::kTutorial) {
		boss_->Update();
	}
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
		Renderer::GetInstance()->DrawModel(Transform::GetInitialValue({ 1.0f,1.0f,1.0f }, { 0.0f,0.0f,0.0f }, { 0.0f,0.0f,-275.0f }), "tutorial_wall", { 1.0f,1.0f,1.0f,1.0f }, false);
		Renderer::GetInstance()->DrawModel(Transform::GetInitialValue({ 1.0f,1.0f,1.0f }, { 0.0f,0.0f,0.0f }, { 0.0f,0.0f,-275.0f }), "tutorial_celling", { 1.0f,1.0f,1.0f,1.0f }, false);

		Renderer::GetInstance()->DrawModel(Transform::GetInitialValue({ 1.0f,1.0f,1.0f }, { 0.0f,0.0f,0.0f }, { 0.0f,0.0f,0.0f }), "wall", { 1.0f,1.0f,1.0f,0.3f }, false);
	} else if (gGamePhase == GamePhase::kBossLastJarona || gGamePhase == GamePhase::kGameClearStage) {
		Renderer::GetInstance()->DrawModel(Transform::GetInitialValue({ 1.0f,1.0f,1.0f }, { 0.0f,0.0f,0.0f }, { 0.0f,0.0f,-275.0f }), "tutorial_wall", { 1.0f,1.0f,1.0f,1.0f }, false);
		Renderer::GetInstance()->DrawModel(Transform::GetInitialValue({ 1.0f,1.0f,1.0f }, { 0.0f,0.0f,0.0f }, { 0.0f,0.0f,-275.0f }), "tutorial_celling", { 1.0f,1.0f,1.0f,1.0f }, false);
	} else if (gGamePhase == GamePhase::kBossLastJaronaAnim) {
		Renderer::GetInstance()->DrawModel(Transform::GetInitialValue({ 1.0f,1.0f,1.0f }, { 0.0f,0.0f,0.0f }, { 0.0f,0.0f,-275.0f }), "tutorial_wall", { 1.0f,1.0f,1.0f,1.0f }, false);
		Renderer::GetInstance()->DrawModel(Transform::GetInitialValue({ 1.0f,1.0f,1.0f }, { 0.0f,0.0f,0.0f }, { 0.0f,0.0f,-275.0f }), "tutorial_celling", { 1.0f,1.0f,1.0f,1.0f }, false);

		Renderer::GetInstance()->DrawModel(Transform::GetInitialValue({ 1.0f,1.0f,1.0f }, { 0.0f,0.0f,0.0f }, { 0.0f,0.0f,0.0f }), "wall", { 1.0f,1.0f,1.0f,1.0f }, false);
	} else {
		Renderer::GetInstance()->DrawModel(Transform::GetInitialValue({ 1.0f,1.0f,1.0f }, { 0.0f,0.0f,0.0f }, { 0.0f,0.0f,0.0f }), "wall", { 1.0f,1.0f,1.0f,1.0f }, false);
		Renderer::GetInstance()->DrawBox(Transform::GetInitialValue({ 10.0f,10.0f,1.0f }, { 0.0f,0.0f,0.0f }, { 0.0f,5.0f,-75.0f }), "door", { 1.0f,1.0f,1.0f,1.0f });
	}

	ProjectileManager::GetInstance()->Draw();


	player_->Draw();
	boss_->Draw();

	TutorialDraw();


	if (gGamePhase != GamePhase::kTutorial && gGamePhase != GamePhase::kBossLastJarona && gGamePhase != GamePhase::kGameClearStage && gGamePhase != GamePhase::kBossLastJaronaAnim) {
		Renderer::GetInstance()->DrawBox(Transform::GetInitialValue({ 10.0f,10.0f,1.0f }, { 0.0f,0.0f,0.0f }, { 0.0f,5.0f,-75.0f }), "door", { 1.0f,1.0f,1.0f,1.0f });
	}

	ParticleManager::GetInstance()->Draw();

	if (gGamePhase != GamePhase::kTutorial) {
		Renderer::GetInstance()->DrawSprite(Transform::GetInitialValue({ 1.0f,1.0f ,1.0f }, { 0.0f,0.0f,0.0f }, { 500.0f,200.0f,0.0f }), "play_guid", { 1.0f,1.0f,1.0f,1.0f });
	}

	fade_->Draw();
	pauseMenu_->Draw();
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

	if (!fade_->isFinished()) {
		return;
	}

	if (InputManager::GetInstance()->IsGamePadConnect()) {
		if (InputManager::GetInstance()->PressPadButton(INPUT_L1) && InputManager::GetInstance()->PressPadButton(INPUT_L1)) {
			fade_->SetColor({ 0.0f,0.0f,0.0f });
			fade_->Start(Fade::Status::FadeOut, 1.0f);
			useSkipStart_ = true;
		}
	} else {
		if (InputManager::GetInstance()->TriggerKey(DIK_O)) {
			fade_->SetColor({ 0.0f,0.0f,0.0f });
			fade_->Start(Fade::Status::FadeOut, 1.0f);
			useSkipStart_ = true;
		}
	}

	AnimSkipFadeUpdate();
	return;
	//}

	//if (InputManager::GetInstance()->IsGamePadConnect()) {
	//	if (InputManager::GetInstance()->TriggerPadButton(INPUT_A)) {
	//		fade_->SetColor({ 0.0f,0.0f,0.0f });
	//		fade_->Start(Fade::Status::FadeOut, 1.0f);
	//		useSkipStart_ = true;
	//	}
	//} else {
	//	if (InputManager::GetInstance()->TriggerKey(DIK_SPACE)) {
	//		fade_->SetColor({ 0.0f,0.0f,0.0f });
	//		fade_->Start(Fade::Status::FadeOut, 1.0f);
	//		useSkipStart_ = true;
	//	}
	//}

	AnimSkipFadeUpdate();
}

void GameScene::AnimSkipFadeUpdate() {
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
				player_->Initialize();
				player_->SetStartPosition({ 0.0f,1.0f,-50.0f });
				boss_->Initialize();
				break;
			case kBossPhaseChangeAnim:
				break;
			case kBossPhase2:
				player_->Initialize();
				player_->SetStartPosition({ 0.0f,1.0f,-50.0f });
				boss_->Initialize();
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
		player_->SetTutorialMinClamp(tutorialAttackWall_->transform_.translate.z);
	} else if (tutorialExitWall_->isActive_) {
		player_->SetTutorialMinClamp(tutorialExitWall_->transform_.translate.z);
	} else {
		player_->SetTutorialMinClamp(80.0f);
	}
}

void GameScene::TutorialDraw() {
	if (gGamePhase != GamePhase::kTutorial) {
		return;
	}

	switch (TutorialManager::GetInstance()->GetCurrentFlagName()) {
	case TutorialManager::TutorialFlagName::kFirstJump:
		Renderer::GetInstance()->DrawSprite(Transform::GetInitialValue({ 1.0f,1.0f,1.0f }, { 0.0f,0.0f,0.0f }, { 0.0f,-200.0f,0.0f }), "jump_to_a", { 1.0f,1.0f,1.0f,1.0f });
		break;
	case TutorialManager::TutorialFlagName::kMoveTest:
		Renderer::GetInstance()->DrawSprite(Transform::GetInitialValue({ 1.0f,1.0f,1.0f }, { 0.0f,0.0f,0.0f }, { 0.0f,-200.0f,0.0f }), "move_to_l", { 1.0f,1.0f,1.0f,1.0f });
		Renderer::GetInstance()->DrawSprite(Transform::GetInitialValue({ 1.0f,1.0f ,1.0f }, { 0.0f,0.0f,0.0f }, { 500.0f,200.0f,0.0f }), "play_guid_tutorial_attack", { 1.0f,1.0f,1.0f,1.0f });
		break;
	case TutorialManager::TutorialFlagName::kAttackTest:
		Renderer::GetInstance()->DrawSprite(Transform::GetInitialValue({ 1.0f,1.0f,1.0f }, { 0.0f,0.0f,0.0f }, { 0.0f,-200.0f,0.0f }), "attack_to_x", { 1.0f,1.0f,1.0f,1.0f });
		Renderer::GetInstance()->DrawSprite(Transform::GetInitialValue({ 1.0f,1.0f ,1.0f }, { 0.0f,0.0f,0.0f }, { 500.0f,200.0f,0.0f }), "play_guid_tutorial_dash", { 1.0f,1.0f,1.0f,1.0f });
		break;
	case TutorialManager::TutorialFlagName::kDashToJump:
		Renderer::GetInstance()->DrawSprite(Transform::GetInitialValue({ 1.0f,1.0f,1.0f }, { 0.0f,0.0f,0.0f }, { 0.0f,-200.0f,0.0f }), "jump_to_a", { 1.0f,1.0f,1.0f,1.0f });
		Renderer::GetInstance()->DrawSprite(Transform::GetInitialValue({ 1.0f,1.0f ,1.0f }, { 0.0f,0.0f,0.0f }, { 500.0f,200.0f,0.0f }), "play_guid", { 1.0f,1.0f,1.0f,1.0f });
		break;
	case TutorialManager::TutorialFlagName::kDashToAttack:
		Renderer::GetInstance()->DrawSprite(Transform::GetInitialValue({ 1.0f,1.0f,1.0f }, { 0.0f,0.0f,0.0f }, { 0.0f,-200.0f,0.0f }), "dash_to_x", { 1.0f,1.0f,1.0f,1.0f });
		Renderer::GetInstance()->DrawSprite(Transform::GetInitialValue({ 1.0f,1.0f ,1.0f }, { 0.0f,0.0f,0.0f }, { 500.0f,200.0f,0.0f }), "play_guid", { 1.0f,1.0f,1.0f,1.0f });
		break;
	case TutorialManager::TutorialFlagName::kDashJumpTest:
		if (player_->GetTransform()->translate.z >= -275.0f) {
			Renderer::GetInstance()->DrawSprite(Transform::GetInitialValue({ 1.0f,1.0f,1.0f }, { 0.0f,0.0f,0.0f }, { 0.0f,-200.0f,0.0f }), "jump_to_a", { 1.0f,1.0f,1.0f,1.0f });
		}
		Renderer::GetInstance()->DrawSprite(Transform::GetInitialValue({ 1.0f,1.0f ,1.0f }, { 0.0f,0.0f,0.0f }, { 500.0f,200.0f,0.0f }), "play_guid", { 1.0f,1.0f,1.0f,1.0f });
		break;
	case TutorialManager::TutorialFlagName::kFinaleTest:
		Renderer::GetInstance()->DrawSprite(Transform::GetInitialValue({ 1.0f,1.0f ,1.0f }, { 0.0f,0.0f,0.0f }, { 500.0f,200.0f,0.0f }), "play_guid", { 1.0f,1.0f,1.0f,1.0f });
		break;
	case TutorialManager::TutorialFlagName::kFinaleAnim:
		Renderer::GetInstance()->DrawSprite(Transform::GetInitialValue({ 1.0f,1.0f ,1.0f }, { 0.0f,0.0f,0.0f }, { 500.0f,200.0f,0.0f }), "play_guid", { 1.0f,1.0f,1.0f,1.0f });
		break;
	default:
		break;
	}

	tutorialAttackWall_->Draw();
	tutorialExitWall_->Draw();
}
