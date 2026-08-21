#include "PauseMenu.h"
void PauseMenu::Initialize() {
	fade_ = std::make_unique<Fade>();
	fade_->Initialize();
	fade_->SetColor({ 0.0f,0.0f,0.0f });
	currentSelect_ = 0;
	startTimer_ = 0.0f;
	finishTimer_ = 0.0f;
	isActive_ = false;
	isFinish_;
	maxSelect_ = 1;
}

void PauseMenu::Update() {
	if (!isActive_) {
		return;
	}
	InputManager* input = InputManager::GetInstance();
	CheckCurrentPhase();

	startTimer_ += DeltaTime::GetInstance()->GetDeltaTime();

	if (startTimer_ < kStartTimerMax) {
		return;
	}

	if (input->TriggerKey(DIK_P) || input->TriggerPadButton(PadButtons::INPUT_START)) {
		DeltaTime::GetInstance()->SetGameTimeSpeed(preGameTimeSpeed_);
		isActive_ = false;
	}

	fade_->Update();

	if (isFinish_) {
		if (fade_->isFinished()) {
			switch (currentSelect_) {
			case 0:
				finishTimer_ += DeltaTime::GetInstance()->GetDeltaTime();

				if (startTimer_ < kStartTimerMax) {
					isActive_ = false;
				}
				break;
			case 1:
				if (maxSelect_ == 2) {
					gGamePhase = GamePhase::kBossLastJaronaAnim;
				} else if (maxSelect_ == 1) {
					gGamePhase = GamePhase::kBossPhaseChangeAnim;
				} else {
					gGamePhase = GamePhase::kGameStartAnim;
				}
				DeltaTime::GetInstance()->SetGameTimeSpeed(preGameTimeSpeed_);
				SceneManager::GetInstance()->ReloadScene();
				break;
			case 2:
				if (maxSelect_ == 2) {
					gGamePhase = GamePhase::kBossPhaseChangeAnim;
				} else if (maxSelect_ == 1) {
					gGamePhase = GamePhase::kGameStartAnim;
				} else {
					gGamePhase = GamePhase::kTutorial;
					DeltaTime::GetInstance()->SetGameTimeSpeed(preGameTimeSpeed_);
					SceneManager::GetInstance()->ChengeScene(SceneName::kTitleScene);
					break;
				}
				DeltaTime::GetInstance()->SetGameTimeSpeed(preGameTimeSpeed_);
				SceneManager::GetInstance()->ReloadScene();
				break;
			case 3:
				if (maxSelect_ == 2) {
					gGamePhase = GamePhase::kGameStartAnim;
				} else {
					gGamePhase = GamePhase::kTutorial;
					DeltaTime::GetInstance()->SetGameTimeSpeed(preGameTimeSpeed_);
					SceneManager::GetInstance()->ChengeScene(SceneName::kTitleScene);
					break;
				}

				DeltaTime::GetInstance()->SetGameTimeSpeed(preGameTimeSpeed_);
				SceneManager::GetInstance()->ReloadScene();
				break;
			case 4:
				gGamePhase = GamePhase::kTutorial;
				DeltaTime::GetInstance()->SetGameTimeSpeed(preGameTimeSpeed_);
				SceneManager::GetInstance()->ChengeScene(SceneName::kTitleScene);
				break;
			}
		}
		return;
	}

	if (TriggerDown()) {
		currentSelect_++;
		if (currentSelect_ > maxSelect_ + 2) {
			currentSelect_ = maxSelect_ + 2;
		}
	}
	if (TriggerUp()) {
		currentSelect_--;

		if (currentSelect_ < 0) {
			currentSelect_ = 0;
		}
	}

	if (TriggerSubmit()) {
		isFinish_ = true;

		switch (currentSelect_) {
		case 0:
			DeltaTime::GetInstance()->SetGameTimeSpeed(preGameTimeSpeed_);
			isActive_ = false;
			break;
		case 1:
		case 2:
		case 3:
		case 4:
			fade_->Start(Fade::Status::FadeOut, 1.0f);
			break;
		default:
			break;
		}
	}

	fade_->Update();
}

void PauseMenu::Draw() {
	if (!isActive_) {
		return;
	}

	Renderer::GetInstance()->DrawSprite(
		Transform::GetInitialValue({ 1.0f,1.0f,1.0f }, { 0.0f,0.0f,0.0f }, { 0.0f,-300.0f,0.0f }),
		"pause",
		{ 1.0f,1.0f,1.0f,1.0f }
	);

	Renderer::GetInstance()->DrawSprite(
		Transform::GetInitialValue({ 1.0f,1.0f,1.0f }, { 0.0f,0.0f,0.0f }, { 0.0f,-150.0f,0.0f }),
		"return_to_game",
		currentSelect_ == 0 ? Vector4{ 1.0f,1.0f,0.0f,1.0f } : Vector4{ 1.0f,1.0f,1.0f,1.0f }
	);

	Renderer::GetInstance()->DrawSprite(
		Transform::GetInitialValue({ 1.0f,1.0f,1.0f }, { 0.0f,0.0f,0.0f }, { 0.0f,-50.0f + (maxSelect_ * 100.0f),0.0f }),
		"retry",
		currentSelect_ == (1 + maxSelect_) ? Vector4{ 1.0f,1.0f,0.0f,1.0f } : Vector4{ 1.0f,1.0f,1.0f,1.0f }
	);

	Renderer::GetInstance()->DrawSprite(
		Transform::GetInitialValue({ 1.0f,1.0f,1.0f }, { 0.0f,0.0f,0.0f }, { 0.0f,50.0f + (maxSelect_ * 100.0f),0.0f }),
		"return_to_title",
		currentSelect_ == (2 + maxSelect_) ? Vector4{ 1.0f,1.0f,0.0f,1.0f } : Vector4{ 1.0f,1.0f,1.0f,1.0f }
	);

	switch (maxSelect_) {
	case 1:
		Renderer::GetInstance()->DrawSprite(
			Transform::GetInitialValue({ 1.0f,1.0f,1.0f }, { 0.0f,0.0f,0.0f }, { 0.0f,-50.0f,0.0f }),
			"retry_phase2",
			currentSelect_ == (maxSelect_) ? Vector4{ 1.0f,1.0f,0.0f,1.0f } : Vector4{ 1.0f,1.0f,1.0f,1.0f }
		);
		break;
	case 2:
		Renderer::GetInstance()->DrawSprite(
			Transform::GetInitialValue({ 1.0f,1.0f,1.0f }, { 0.0f,0.0f,0.0f }, { 0.0f,50.0f,0.0f }),
			"retry_phase2",
			currentSelect_ == (maxSelect_) ? Vector4{ 1.0f,1.0f,0.0f,1.0f } : Vector4{ 1.0f,1.0f,1.0f,1.0f }
		);
		Renderer::GetInstance()->DrawSprite(
			Transform::GetInitialValue({ 1.0f,1.0f,1.0f }, { 0.0f,0.0f,0.0f }, { 0.0f,-50.0f,0.0f }),
			"retry_last_jarona",
			currentSelect_ == (maxSelect_ - 1) ? Vector4{ 1.0f,1.0f,0.0f,1.0f } : Vector4{ 1.0f,1.0f,1.0f,1.0f }
		);
		break;
	default:
		break;
	}

	Renderer::GetInstance()->DrawSprite(
		Transform::GetInitialValue({ 1.0f,1.0f,1.0f }, { 0.0f,0.0f,0.0f }, { 0.0f,0.0f,0.0f }), Vector2(1280.0f, 720.0f),
		"white_template",
		{ 0.0f,0.0f,0.0f,Easing(0.0f,0.5f,startTimer_,kStartTimerMax,EaseType::kEaseIn) }
	);

	fade_->Draw();
}

void PauseMenu::ShowMenu() {
	isActive_ = true;
	isFinish_ = false;
	currentSelect_ = 0;
	startTimer_ = 0.0f;
	finishTimer_ = 0.0f;
	preGameTimeSpeed_ = DeltaTime::GetInstance()->GetGameTimeSpeed();
	DeltaTime::GetInstance()->SetGameTimeSpeed(0.0f);
}

void PauseMenu::CheckCurrentPhase() {
	switch (gGamePhase) {
	case kTutorial:
		break;
	case kGameStartAnim:
	case kBossPhase1:
		break;
	case kBossPhaseChangeAnim:
	case kBossPhase2:
		break;
	case kBossLastJaronaAnim:
	case kBossLastJarona:
	case kGameClearStage:
		break;
	default:
		break;
	}
}
bool PauseMenu::TriggerUp() {
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

bool PauseMenu::TriggerDown() {
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

bool PauseMenu::TriggerSubmit() {
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