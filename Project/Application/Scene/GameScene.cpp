#include "GameScene.h"
#include "Satlib.h"
#include "../ProjectileManager.h"

GameScene::~GameScene() {
}

void GameScene::Initialize() {
	SoundManager::GetInstance()->ResetBGM();

	DeltaTime::GetInstance()->SetGameTimeSpeed(1.0f);
	LightManager::GetInstance()->GetDirectionalLightData()->intensity = 0.15f;
	LightManager::GetInstance()->GetDirectionalLightData()->direction = { 0.0f,-1.0f,0.0f };
	LightManager::GetInstance()->GetDirectionalLightData()->color = { 1.0f,1.0f,1.0f,1.0f };
	ParticleManager::GetInstance()->ClearParticles();

	LightManager::GetInstance()->ClearLight();
	Camera::GetInstance()->SetPosition({ 0.0f,2.0f,-30.0f });
	ground_ = std::make_unique<Ground>();
	ground_->Initialize();

	fade_ = std::make_unique<Fade>();
	fade_->Initialize();
	fade_->SetColor({ 0.0f,0.0f,0.0f });
	fade_->Start(Fade::Status::FadeIn, 1.0f);
}

void GameScene::Update() {
#ifdef _DEBUG

	if (InputManager::GetInstance()->TriggerKey(DIK_R)) {
		SceneManager::GetInstance()->ReloadScene();
	}

	if (InputManager::GetInstance()->TriggerKey(DIK_F4)) {
		Renderer::GetInstance()->ChangeUseDebugLine();
	}
#endif // _DEBUG

	Camera::GetInstance()->Update();
	ParticleManager::GetInstance()->Update();
	fade_->Update();
}

void GameScene::Draw() {
	ground_->Draw();

	Renderer::GetInstance()->DrawModel(
		Transform::GetInitialValue(),
		"halberd", {1.0f,1.0f,1.0f,1.0f},false);

	fade_->Draw();
}
