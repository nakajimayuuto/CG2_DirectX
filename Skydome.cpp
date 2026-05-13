#include "Skydome.h"

void Skydome::Initialize() {
	model_.Initialize(ModelManager::GetInstance()->GetModelInfo("skydome"));
	model_.SetLightingType(Renderer::LightingType::kNone);
}

void Skydome::Update() {

}

void Skydome::Draw() {
	model_.Draw({ {1.0f,1.0f,1.0f} ,{0.0f,0.0f,0.0f} ,{0.0f,0.0f,0.0f} });
}