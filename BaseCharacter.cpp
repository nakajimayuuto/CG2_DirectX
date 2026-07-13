#include "BaseCharacter.h"

void BaseCharacter::Initialize() {
	transform_.Initialize();
}

void BaseCharacter::Update() {

}

void BaseCharacter::Draw() {
	for (std::pair<std::string, Model> model : models_) {
		Renderer::GetInstance()->DrawModel(transform_.GetAffineMatrix(), &model.second);
	}
}