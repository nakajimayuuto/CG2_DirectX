#include "Skydome.h"

void Skydome::Initialize() {
	//transform_.Initialize();
	//model_.Initialize(ModelManager::GetInstance()->GetModelInfo("skydome"));
	//model_.SetLightingType(LightingType::kNone);
	//
	//color_ = { 1.0f,1.0f,1.0f,1.0f };
}

void Skydome::Update() {
#ifdef _DEBUG


	ImGui::Begin("DirectionalLight");

	ImGui::DragFloat("intensity",&LightManager::GetInstance()->GetDirectionalLightData()->intensity,0.01f,0.0f,1.0f);

	ImGui::End();
#endif // _DEBUG

	//color_ = {
	//LightManager::GetInstance()->GetDirectionalLightData()->intensity,
	//LightManager::GetInstance()->GetDirectionalLightData()->intensity,
	//LightManager::GetInstance()->GetDirectionalLightData()->intensity,
	//1.0f
	//};
	//
	//model_.SetColor(color_);
}

void Skydome::Draw() {
	//Renderer::GetInstance()->DrawModel(transform_, &model_,false);
}