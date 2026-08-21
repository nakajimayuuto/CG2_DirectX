#include "TitleScene.h"

void TitleScene::Initialize() {
	LightManager::GetInstance()->GetDirectionalLightData()->intensity = 0.05f;
	LightManager::GetInstance()->GetDirectionalLightData()->color = { 0.5f,0.5f,1.0f,1.0f };
	LightManager::GetInstance()->ClearLight();

	ground_ = std::make_unique<Ground>();
	ground_->Initialize();

	fade_ = std::make_unique<Fade>();
	fade_->Initialize();
	fade_->SetColor({ 0.0f,0.0f,0.0f });
	fade_->Start(Fade::Status::FadeIn, 1.0f);


	worldFrameEmitter_ = std::make_unique<Emitter>();
	worldFrameEmitter_->SetParticle(ParticleManager::GetInstance()->GetParticles("big_blue_fire"));
	// 第一形態.
	worldFrameEmitter_->Initialize(Transform::GetInitialValue({ 150.0f,10.0f,150.0f }, { 0.0f,0.0f,0.0f }, { 0.0f,5.0f,0.0f }), 10, 0.01f);
	// 第二形態.
	//worldFrameEmitter_->Initialize(Transform::GetInitialValue({ 150.0f,10.0f,150.0f }, { 0.0f,0.0f,0.0f }, { 0.0f,5.0f,0.0f }), 30, 0.1f);

	//worldBigFrameEmitter_ = std::make_unique<Emitter>();
	//worldBigFrameEmitter_->SetParticle(ParticleManager::GetInstance()->GetParticles("big_fire"));
	//worldBigFrameEmitter_->Initialize(Transform::GetInitialValue({ 150.0f,10.0f,150.0f }, { 0.0f,0.0f,0.0f }, { 0.0f,5.0f,0.0f }), 5, 0.2f);
	center_.Initialize();

	cameraTransform_.Initialize();
	cameraTransform_.translate.y = 15.0f;
	cameraTransform_.translate.z = -50.0f;
	cameraTransform_.SetParent(&center_);

	currentPhase_ = TitlePhase::kTitle;

	for (size_t i = 0; i < static_cast<size_t>(TitlePhase::kCount); i++) {
		if (static_cast<TitlePhase>(i) == currentPhase_) {
			easeTimer_[static_cast<size_t>(i)] = 0.0f;
		} else {
			easeTimer_[static_cast<size_t>(i)] = 1.0f;
		}
	}

	sndSelect_ = SoundManager::GetInstance()->GetSoundData("snd_select");

	LightManager::GetInstance()->CreatePointLight("halberd_title_light");
	LightManager::GetInstance()->GetLightData("halberd_title_light")->color = { 1.0f,1.0f,1.0f,1.0f };
	LightManager::GetInstance()->GetLightData("halberd_title_light")->radius = 10.0f;
	LightManager::GetInstance()->GetLightData("halberd_title_light")->intensity = 1.0f;
	LightManager::GetInstance()->GetLightData("halberd_title_light")->position = { 0.0f,5.0f,0.0f };

	currentMenuSelectNum_ = 0;
	currentDifficultySelectNum_ = 0;

	preStickUpUse_ = false;
	preStickDownUse_ = false;

	isSubmit_ = false;
}

void TitleScene::Update() {
	if (InputManager::GetInstance()->TriggerKey(DIK_F3)) {
		Camera::GetInstance()->ChangeCameraMode();
	}
	if (InputManager::GetInstance()->TriggerKey(DIK_R)) {
		SceneManager::GetInstance()->ReloadScene();
	}
	center_.rotate.y += DeltaTime::GetInstance()->GetDeltaTime() * kRotateAngleSpeed;
	if (center_.rotate.y >= Radian(360.0f)) {
		center_.rotate.y -= Radian(360.0f);
	}

	if (isSubmit_) {
		if (fade_->isFinished()) {
			SceneManager::GetInstance()->ChengeScene(SceneName::kGameScene);
		}
	} else {
		PhaseSelect();
	}

	fade_->Update();

	worldFrameEmitter_->Update();
	//worldBigFrameEmitter_->Update();
	Camera::GetInstance()->SetPosition(cameraTransform_.GetWorldPosition());
	Vector2 rotateVector = { cameraTransform_.GetWorldPosition().x,cameraTransform_.GetWorldPosition().z };
	Camera::GetInstance()->SetRotate({ Radian(15.0f),-VectorToRadian(rotateVector) - Radian(90.0f),0.0f });
	Camera::GetInstance()->Update();

}

void TitleScene::PhaseSelect() {
	switch (currentPhase_) {
	case TitlePhase::kTitle:
		TitleUpdate();
		break;
	case TitlePhase::kMenuSelect:
		MenuUpdate();
		break;
	case TitlePhase::kDifficultySelect:
		DiffucltyUpdate();
		break;
	case TitlePhase::kSetting:
		SettingUpdate();
		break;
	default:
		break;
	}

	EaseTimerUpdate(currentPhase_);
}

void TitleScene::TitleUpdate() {
	if (TriggerSubmit()) {
		SoundManager::GetInstance()->SoundPlay(sndSelect_, 1.0f, 0.5f, SoundType::kSoundEffect);
		InputManager::GetInstance()->SetVibration(0.1f, 0.1f, 0.5f);
		currentMenuSelectNum_ = 0;
		//currentPhase_ = TitlePhase::kMenuSelect;
		currentPhase_ = TitlePhase::kDifficultySelect;
	}
}

void TitleScene::MenuUpdate() {
	if (TriggerDown()) {
		currentMenuSelectNum_++;
		SoundManager::GetInstance()->SoundPlay(sndSelect_, 1.0f, 0.5f, SoundType::kSoundEffect);
		if (currentMenuSelectNum_ > 2) {
			currentMenuSelectNum_ = 2;
		}
	}
	if (TriggerUp()) {
		currentMenuSelectNum_--;

		SoundManager::GetInstance()->SoundPlay(sndSelect_, 1.0f, 0.5f, SoundType::kSoundEffect);
		if (currentMenuSelectNum_ < 0) {
			currentMenuSelectNum_ = 0;
		}
	}

	switch (currentMenuSelectNum_) {
	case 0:
		if (TriggerSubmit()) {
			currentDifficultySelectNum_ = static_cast<int32_t>(DifficultyManager::GetInstance()->GetCurrentDifficulty());
			currentPhase_ = TitlePhase::kDifficultySelect;
			SoundManager::GetInstance()->SoundPlay(sndSelect_, 1.0f, 0.5f, SoundType::kSoundEffect);
		}
		break;
	case 1:
		break;
	case 2:
		if (TriggerSubmit()) {
			currentPhase_ = TitlePhase::kTitle;
			SoundManager::GetInstance()->SoundPlay(sndSelect_, 1.0f, 0.5f, SoundType::kSoundEffect);
		}
		break;
	default:
		break;
	}
}

void TitleScene::DiffucltyUpdate() {
	if (TriggerDown()) {
		currentDifficultySelectNum_++;
		SoundManager::GetInstance()->SoundPlay(sndSelect_, 1.0f, 0.5f, SoundType::kSoundEffect);

		if (currentDifficultySelectNum_ > 3) {
			currentDifficultySelectNum_ = 3;
		}
	}
	if (TriggerUp()) {
		currentDifficultySelectNum_--;
		SoundManager::GetInstance()->SoundPlay(sndSelect_, 1.0f, 0.5f, SoundType::kSoundEffect);

		if (currentDifficultySelectNum_ < 0) {
			currentDifficultySelectNum_ = 0;
		}
	}

	if (TriggerSubmit()) {
		InputManager::GetInstance()->SetVibration(0.1f, 0.1f, 0.5f);
		SoundManager::GetInstance()->SoundPlay(sndSelect_, 1.0f, 0.5f, SoundType::kSoundEffect);
		switch (currentDifficultySelectNum_) {
		case 0:
			DifficultyManager::GetInstance()->SetCurrentDifficulty(Difficulty::kDifficultyEasy);
			break;
		case 1:
			DifficultyManager::GetInstance()->SetCurrentDifficulty(Difficulty::kDifficultyNormal);
			break;
		case 2:
			DifficultyManager::GetInstance()->SetCurrentDifficulty(Difficulty::kDifficultyHard);
			break;
		case 3:
			currentMenuSelectNum_ = 0;
			//currentPhase_ = TitlePhase::kMenuSelect;
			currentPhase_ = TitlePhase::kTitle;
			return;
			break;
		default:
			break;
		}
		isSubmit_ = true;
		fade_->Start(Fade::Status::FadeOut, 1.0f);
	}
}

void TitleScene::SettingUpdate() {
}

void TitleScene::Draw() {
	ground_->Draw();

	Renderer::GetInstance()->DrawModel(Transform::GetInitialValue({ 1.0f,1.0f,1.0f }, { 0.0f,0.0f,0.0f }, { 0.0f,0.0f,0.0f }), "wall", { 1.0f,1.0f,1.0f,1.0f }, false);
	Renderer::GetInstance()->DrawBox(Transform::GetInitialValue({ 10.0f,10.0f,1.0f }, { 0.0f,0.0f,0.0f }, { 0.0f,5.0f,-75.0f }), "door", { 1.0f,1.0f,1.0f,1.0f });

	Renderer::GetInstance()->DrawModel(Transform::GetInitialValue({ 1.0f,1.0f,1.0f }, { 0.0f,0.0f,0.0f }, { 0.0f,2.0f,0.0f }), "halberd", { 1.0f,1.0f,1.0f,1.0f }, false);
	Renderer::GetInstance()->DrawShadow(Transform::GetInitialValue({ 1.0f,1.0f,1.0f }, { 0.0f,0.0f,0.0f }, { 0.0f,0.0f,0.0f }), "halberd", { 0.0f,0.0f,0.0f,1.0f });

	ParticleManager::GetInstance()->Draw();

	Renderer::GetInstance()->DrawSprite(
		Transform::GetInitialValue({ 1.0f,1.0f ,1.0f }, { 0.0f,0.0f,Radian(-5.0f) },
			{ Easing(kMenuPosX,-1100.0f,kUIAnimEaseTimerMax_ - easeTimer_[static_cast<size_t>(TitlePhase::kTitle)],kUIAnimEaseTimerMax_,EaseType::kEaseIn) - 100.0f,0.0f,0.0f }
		)
		, Vector2(640.0f, 1000.0f), "white_template", { 0.025f,0.025f,0.025f,1.0f });

	Renderer::GetInstance()->DrawSprite(
		Transform::GetInitialValue({ 1.0f,1.0f ,1.0f }, { 0.0f,0.0f,Radian(-5.0f) },
			{ Easing(kMenuPosX,-1100.0f,kUIAnimEaseTimerMax_ - easeTimer_[static_cast<size_t>(TitlePhase::kTitle)],kUIAnimEaseTimerMax_,EaseType::kEaseIn) + 230.0f,0.0f,0.0f }
		)
		, Vector2(20.0f, 1000.0f), "white_template", { 0.0f,0.0f,0.0f,1.0f });

	Renderer::GetInstance()->DrawSprite(Transform::GetInitialValue({ 1.0f,1.0f ,1.0f }, { 0.0f,0.0f,0.0f }, { 0.0f,335.0f,0.0f }), Vector2(1280.0f, 50.0f), "white_template", { 0.0f,0.0f,0.0f,1.0f });
	Renderer::GetInstance()->DrawSprite(Transform::GetInitialValue({ 1.0f,1.0f ,1.0f }, { 0.0f,0.0f,0.0f }, { 0.0f,-335.0f,0.0f }), Vector2(1280.0f, 50.0f), "white_template", { 0.0f,0.0f,0.0f,1.0f });

	// pressA
	Renderer::GetInstance()->DrawSprite(Transform::GetInitialValue({ 1.0f,1.0f ,1.0f }, { 0.0f,0.0f,0.0f }, { 0.0f,Easing(kPressAPosY,460.0f,easeTimer_[static_cast<size_t>(TitlePhase::kTitle)],kUIAnimEaseTimerMax_,EaseType::kEaseIn),0.0f }), "press_a", { 1.0f,1.0f,1.0f,1.0f });

	// difficulty
	Renderer::GetInstance()->DrawSprite(
		Transform::GetInitialValue({ 1.0f,1.0f ,1.0f }, { 0.0f,0.0f,0.0f },
			{ Easing(kMenuPosX,-1100.0f,easeTimer_[static_cast<size_t>(TitlePhase::kMenuSelect)],kUIAnimEaseTimerMax_,EaseType::kEaseIn),0.0f,0.0f }
		),
		"menu_start", (currentMenuSelectNum_ == 0 ? Vector4(1.0f, 1.0f, 0.0f, 1.0f) : Vector4(1.0f, 1.0f, 1.0f, 1.0f)));

	Renderer::GetInstance()->DrawSprite(
		Transform::GetInitialValue({ 1.0f,1.0f ,1.0f }, { 0.0f,0.0f,0.0f },
			{ Easing(kMenuPosX,-1100.0f,easeTimer_[static_cast<size_t>(TitlePhase::kMenuSelect)],kUIAnimEaseTimerMax_,EaseType::kEaseIn),100.0f,0.0f }
		),
		"menu_setting", (currentMenuSelectNum_ == 1 ? Vector4(1.0f, 1.0f, 0.0f, 1.0f) : Vector4(1.0f, 1.0f, 1.0f, 1.0f)));
	Renderer::GetInstance()->DrawSprite(
		Transform::GetInitialValue({ 1.0f,1.0f ,1.0f }, { 0.0f,0.0f,0.0f },
			{ Easing(kMenuPosX,-1100.0f,easeTimer_[static_cast<size_t>(TitlePhase::kMenuSelect)],kUIAnimEaseTimerMax_,EaseType::kEaseIn),200.0f,0.0f }
		),
		"menu_return", (currentMenuSelectNum_ == 2 ? Vector4(1.0f, 1.0f, 0.0f, 1.0f) : Vector4(1.0f, 1.0f, 1.0f, 1.0f)));


	Renderer::GetInstance()->DrawSprite(
		Transform::GetInitialValue({ 1.0f,1.0f ,1.0f }, { 0.0f,0.0f,0.0f },
			{ Easing(kMenuPosX,-1100.0f,easeTimer_[static_cast<size_t>(TitlePhase::kDifficultySelect)],kUIAnimEaseTimerMax_,EaseType::kEaseIn),-250.0f,0.0f }
		),
		"ui_difficulty", Vector4(1.0f, 1.0f, 1.0f, 1.0f));

	Renderer::GetInstance()->DrawSprite(
		Transform::GetInitialValue({ 1.0f,1.0f ,1.0f }, { 0.0f,0.0f,0.0f },
			{ Easing(kMenuPosX,-1100.0f,easeTimer_[static_cast<size_t>(TitlePhase::kDifficultySelect)],kUIAnimEaseTimerMax_,EaseType::kEaseIn),-100.0f,0.0f }
		),
		"difficulty_easy", (currentDifficultySelectNum_ == 0 ? Vector4(1.0f, 1.0f, 0.0f, 1.0f) : Vector4(1.0f, 1.0f, 1.0f, 1.0f)));

	Renderer::GetInstance()->DrawSprite(
		Transform::GetInitialValue({ 1.0f,1.0f ,1.0f }, { 0.0f,0.0f,0.0f },
			{ Easing(kMenuPosX,-1100.0f,easeTimer_[static_cast<size_t>(TitlePhase::kDifficultySelect)],kUIAnimEaseTimerMax_,EaseType::kEaseIn),0.0f,0.0f }
		),
		"difficulty_normal", (currentDifficultySelectNum_ == 1 ? Vector4(1.0f, 1.0f, 0.0f, 1.0f) : Vector4(1.0f, 1.0f, 1.0f, 1.0f)));
	Renderer::GetInstance()->DrawSprite(
		Transform::GetInitialValue({ 1.0f,1.0f ,1.0f }, { 0.0f,0.0f,0.0f },
			{ Easing(kMenuPosX,-1100.0f,easeTimer_[static_cast<size_t>(TitlePhase::kDifficultySelect)],kUIAnimEaseTimerMax_,EaseType::kEaseIn),100.0f,0.0f }
		),
		"difficulty_hard", (currentDifficultySelectNum_ == 2 ? Vector4(1.0f, 1.0f, 0.0f, 1.0f) : Vector4(1.0f, 1.0f, 1.0f, 1.0f)));
	Renderer::GetInstance()->DrawSprite(
		Transform::GetInitialValue({ 1.0f,1.0f ,1.0f }, { 0.0f,0.0f,0.0f },
			{ Easing(kMenuPosX,-1100.0f,easeTimer_[static_cast<size_t>(TitlePhase::kDifficultySelect)],kUIAnimEaseTimerMax_,EaseType::kEaseIn),200.0f,0.0f }
		),
		"ui_return", (currentDifficultySelectNum_ == 3 ? Vector4(1.0f, 1.0f, 0.0f, 1.0f) : Vector4(1.0f, 1.0f, 1.0f, 1.0f)));

	Renderer::GetInstance()->DrawSprite(
		Transform::GetInitialValue({ 1.0f,1.0f ,1.0f }, { 0.0f,0.0f,0.0f },
			{ Easing(kMenuPosX,-1100.0f,easeTimer_[static_cast<size_t>(TitlePhase::kDifficultySelect)],kUIAnimEaseTimerMax_,EaseType::kEaseIn),0.0f,0.0f }
		),
		"difficulty_normal_outline", Vector4(1.0f, 1.0f, 1.0f, 1.0f));
	Renderer::GetInstance()->DrawSprite(
		Transform::GetInitialValue({ 1.0f,1.0f ,1.0f }, { 0.0f,0.0f,0.0f },
			{ Easing(kMenuPosX,-1100.0f,easeTimer_[static_cast<size_t>(TitlePhase::kDifficultySelect)],kUIAnimEaseTimerMax_,EaseType::kEaseIn),100.0f,0.0f }
		),
		"difficulty_hard_outline", Vector4(1.0f, 1.0f, 1.0f, 1.0f));

	fade_->Draw();
}

bool TitleScene::TriggerUp() {
	if (InputManager::GetInstance()->IsGamePadConnect()) {
		if (InputManager::GetInstance()->TriggerPadButton(PadButtons::INPUT_UP) || InputManager::GetInstance()->TriggerPadButton(PadButtons::INPUT_LSTICK_UP)) {
			return true;
		}
	} else {
		if (InputManager::GetInstance()->TriggerKey(DIK_UP) || InputManager::GetInstance()->TriggerKey(DIK_W)) {
			return true;
		}
	}

	return false;
}

bool TitleScene::TriggerDown() {
	if (InputManager::GetInstance()->IsGamePadConnect()) {
		if (InputManager::GetInstance()->TriggerPadButton(PadButtons::INPUT_DOWN) || InputManager::GetInstance()->TriggerPadButton(PadButtons::INPUT_LSTICK_DOWN)) {
			return true;
		}
	} else {
		if (InputManager::GetInstance()->TriggerKey(DIK_DOWN) || InputManager::GetInstance()->TriggerKey(DIK_S)) {
			return true;
		}
	}

	return false;
}

bool TitleScene::TriggerSubmit() {
	if (InputManager::GetInstance()->IsGamePadConnect()) {
		if (InputManager::GetInstance()->TriggerPadButton(PadButtons::INPUT_A) || InputManager::GetInstance()->TriggerPadButton(PadButtons::INPUT_B)) {
			return true;
		}
	} else {
		if (InputManager::GetInstance()->TriggerKey(DIK_SPACE) || InputManager::GetInstance()->TriggerKey(DIK_Z)) {
			return true;
		}
	}

	return false;
}

void TitleScene::EaseTimerUpdate(TitlePhase incrimentTimer) {
	for (size_t i = 0; i < static_cast<size_t>(TitlePhase::kCount); i++) {
		if (static_cast<TitlePhase>(i) == incrimentTimer) {
			if (easeTimer_[static_cast<size_t>(i)] <= 0.0f) {
				easeTimer_[static_cast<size_t>(i)] = 0.0f;
			} else {
				easeTimer_[static_cast<size_t>(i)] -= DeltaTime::GetInstance()->GetDeltaTime();
			}
		} else {
			if (easeTimer_[static_cast<size_t>(i)] >= kUIAnimEaseTimerMax_) {
				easeTimer_[static_cast<size_t>(i)] = kUIAnimEaseTimerMax_;
			} else {
				easeTimer_[static_cast<size_t>(i)] += DeltaTime::GetInstance()->GetDeltaTime();
			}
		}
	}
}
