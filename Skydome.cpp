#include "Skydome.h"

void Skydome::Initialize() {
	model_.Initialize(ModelManager::GetInstance()->GetModelInfo("skydome"));
	model_.SetLightingType(Renderer::LightingType::kNone);
}

void Skydome::Update() {

}

void Skydome::Draw() {
	model_.Draw(Transform::GetInitialValue());
}