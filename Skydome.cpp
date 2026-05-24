#include "Skydome.h"

void Skydome::Initialize() {
	transform_.Initialize();
	model_.Initialize(ModelManager::GetInstance()->GetModelInfo("skydome"));
	model_.SetLightingType(Renderer::LightingType::kNone);
}

void Skydome::Draw() {
	model_.Draw(transform_);
}