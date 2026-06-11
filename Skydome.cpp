#include "Skydome.h"

void Skydome::Initialize() {
	transform_.Initialize();
	//model_.Initialize(ModelManager::GetInstance()->GetModelInfo("skydome"));
	//model_.SetLightingType(LightingType::kAspectNone);
}

void Skydome::Draw() {
	//model_.Draw(transform_);
}