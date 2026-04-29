#include "Skydome.h"
void Skydome::Initialize() {
	model_.Initialize(ModelManager::GetInstance()->GetModelInfo("skydome"));
	transform_.Initialize();
}

void Skydome::Update() {

}

void Skydome::Draw() {
	model_.Draw(transform_);
}