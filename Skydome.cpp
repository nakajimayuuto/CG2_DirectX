#include "Skydome.h"

void Skydome::Initialize() {
	transform_.Initialize();
	model_.Initialize(ModelManager::GetInstance()->GetModelInfo("skydome"));
	model_.SetLightingType(LightingType::kNone);
}

void Skydome::Draw() {
	Renderer::GetInstance()->DrawModel(transform_, &model_);
}