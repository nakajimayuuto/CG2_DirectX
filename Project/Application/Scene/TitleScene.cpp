#include "TitleScene.h"

void TitleScene::Initialize() {
	SoundManager::GetInstance()->ResetBGM();
	//SoundManager::GetInstance()->SoundPlay("mus_title", 1.0f, 0.25f, kBGM, true, "mus_title");
	//LightManager::GetInstance()->GetDirectionalLightData()->intensity = 0.05f;
	//LightManager::GetInstance()->GetDirectionalLightData()->color = { 0.5f,0.5f,1.0f,1.0f };
	LightManager::GetInstance()->GetDirectionalLightData()->intensity = 1.0f;
	LightManager::GetInstance()->GetDirectionalLightData()->color = { 1.0f,1.0f,1.0f,1.0f };
	LightManager::GetInstance()->ClearLight();

	fade_ = std::make_unique<Fade>();
	fade_->Initialize();
	fade_->SetColor({ 0.0f,0.0f,0.0f });
	fade_->Start(Fade::Status::FadeIn, 1.0f);

	//LightManager::GetInstance()->CreateSpotLight("halberd_title_light");
	//LightManager::GetInstance()->GetLightData("halberd_title_light")->color = { 1.0f,1.0f,1.0f,1.0f };
	//LightManager::GetInstance()->GetLightData("halberd_title_light")->radius = 10.0f;
	//LightManager::GetInstance()->GetLightData("halberd_title_light")->intensity = 1.0f;
	//LightManager::GetInstance()->GetLightData("halberd_title_light")->position = { 0.0f,6.0f,0.0f };
	//LightManager::GetInstance()->GetLightData("halberd_title_light")->direction = { 0.0f,-1.0f,0.0f };
	//LightManager::GetInstance()->GetLightData("halberd_title_light")->distance = 1.0f;

}

void TitleScene::Update() {
	fade_->Update();

	Camera::GetInstance()->Update();
}

void TitleScene::Draw() {

	fade_->Draw();
}