#include "TitleScene.h"

void TitleScene::Initialize() {
	ModelManager::GetInstance()->RegisterObj("title", "Resource/Title", "Title.obj");
	ModelManager::GetInstance()->RegisterObj("player", "Resource/player", "player.obj");

	titleModel_.Initialize(ModelManager::GetInstance()->GetModelInfo("title"));
	titleTransform_.Initialize();
	titleTransform_.translate = { 0.0f,2.0f,0.0f };
	titleTransform_.rotate.x = Radian(-90.0f);

	playerModel_.Initialize(ModelManager::GetInstance()->GetModelInfo("player"));
	playerTransform_.Initialize();
	playerTransform_.rotate.y = Radian(-210.0f);

	Camera::GetInstance()->Initialize();
	Camera::GetInstance()->SetPosition({ 0.0f,0.0f,-15.0f });

	fade_ = new Fade();
	fade_->Initialize();
	fade_->Start(Fade::Status::FadeIn, 1.0f);

	phase_ = Phase::kFadeIn;
}

void TitleScene::Update() {

	switch (phase_) {
	case TitleScene::Phase::kFadeIn:

		fade_->Update();

		if (fade_->GetIsFinished()) {
			phase_ = Phase::kMain;
		}

		Camera::GetInstance()->Update();
		break;
	case TitleScene::Phase::kMain:
		if (InputManager::GetInstance()->TriggerKey(DIK_SPACE)) {
			fade_->Start(Fade::Status::FadeOut, 1.0f);
			phase_ = Phase::kFadeOut;
		}

		Camera::GetInstance()->Update();
		break;
	case TitleScene::Phase::kFadeOut:

		if (fade_->GetIsFinished()) {
			SceneManager::GetInstance()->ChengeScene(SceneName::kGameScene);
		}

		fade_->Update();
		Camera::GetInstance()->Update();
		break;
	default:
		break;
	}

}

void TitleScene::Draw() {
	titleModel_.Draw(titleTransform_);
	playerModel_.Draw(playerTransform_);

	fade_->Draw();
}
